#include <stdafx.h>

#include <reader.h>
#include <value.h>
#include <utility>
#include <cstdio>
#include <cassert>
#include <cstring>
#include <iostream>
#include <stdexcept>

#if _MSC_VER >= 1400 // VC++ 8.0
#pragma warning( disable : 4996 )   // disable warning about strdup being deprecated.
#endif

namespace Json {

// Implementation of class Features
// ////////////////////////////////

Features::Features()
   : allowComments_( true )
   , strictRoot_( false )
{
}

Features 
Features::all()
{
   return Features();
}

Features 
Features::strictMode()
{
   Features features;
   features.allowComments_ = false;
   features.strictRoot_ = true;
   return features;
}

// Implementation of class Reader
// ////////////////////////////////


static inline bool in( Reader::Char c, Reader::Char c1, Reader::Char c2, Reader::Char c3, Reader::Char c4 )
{
   return c == c1  ||  c == c2  ||  c == c3  ||  c == c4;
}

static inline bool in( Reader::Char c, Reader::Char c1, Reader::Char c2, Reader::Char c3, Reader::Char c4, Reader::Char c5 )
{
   return c == c1  ||  c == c2  ||  c == c3  ||  c == c4  ||  c == c5;
}

static bool containsNewLine( Reader::Location begin, Reader::Location end )
{
   for ( ;begin < end; ++begin )
   {
      if ( *begin == _T('\n')  ||  *begin == _T('\r') )
      {
         return true;
      }
   }

   return false;
}

static _string codePointToUTF8( unsigned int cp )
{
   _string result;
   
   // based on description from http://en.wikipedia.org/wiki/UTF-8

   if( cp <= 0x7f )
   {
      result.resize( 1 );
      result[0] = static_cast<TCHAR>( cp );
   } 
   else if( cp <= 0x7FF )
   {
      result.resize( 2 );
      result[1] = static_cast<TCHAR>( 0x80 | ( 0x3f & cp ) );
      result[0] = static_cast<TCHAR>( 0xC0 | ( 0x1f & ( cp >> 6 ) ) );
   } 
   else if( cp <= 0xFFFF )
   {
      result.resize( 3 );
      result[2] = static_cast<TCHAR>( 0x80 | ( 0x3f & cp ) );
      result[1] = 0x80 | static_cast<TCHAR>( ( 0x3f & ( cp >> 6 ) ) );
      result[0] = 0xE0 | static_cast<TCHAR>( ( 0xf & ( cp >> 12 ) ));
   }
   else if( cp <= 0x10FFFF )
   {
      result.resize( 4 );
      result[3] = static_cast<TCHAR>( 0x80 | ( 0x3f & cp ) );
      result[2] = static_cast<TCHAR>( 0x80 | ( 0x3f & ( cp >> 6 ) ) );
      result[1] = static_cast<TCHAR>( 0x80 | ( 0x3f & ( cp >> 12 ) ) );
      result[0] = static_cast<TCHAR>( 0xF0 | ( 0x7 & ( cp >> 18 ) ) );
   }

   return result;
}


// Class Reader
// //////////////////////////////////////////////////////////////////

Reader::Reader()
   : features_( Features::all() )
{
}


Reader::Reader( const Features &features )
   : features_( features )
{
}


bool Reader::parse( const _string &document, Value &root, bool collectComments )
{
   document_ = document;
   const TCHAR *begin = document_.c_str();
   const TCHAR *end = begin + document_.length();
   return parse( begin, end, root, collectComments );
}

bool Reader::parse( std::_tistream& sin, Value &root, bool collectComments )
{
   //std::istream_iterator<char> begin(sin);
   //std::istream_iterator<char> end;
   // Those would allow streamed input from a file, if parse() were a
   // template function.

   // Since std::string is reference-counted, this at least does not
   // create an extra copy.
   _string doc;
   std::getline(sin, doc, (TCHAR)EOF);
   return parse( doc, root, collectComments );
}

bool Reader::parse( const TCHAR *beginDoc, const TCHAR *endDoc, Value &root, bool collectComments )
{
   if ( !features_.allowComments_ )
   {
      collectComments = false;
   }

   begin_ = beginDoc;
   end_ = endDoc;
   collectComments_ = collectComments;
   current_ = begin_;
   lastValueEnd_ = 0;
   lastValue_ = 0;
   commentsBefore_ = _T("");
   errors_.clear();
   
   while ( !nodes_.empty() )
   {
      nodes_.pop();
   }

   nodes_.push( &root );
   
   bool successful = readValue();
   Token token;
   skipCommentTokens( token );
   
   if ( collectComments_  &&  !commentsBefore_.empty() )
   {
      root.setComment( commentsBefore_, commentAfter );
   }
   
   if ( features_.strictRoot_ )
   {
      if ( !root.isArray()  &&  !root.isObject() )
      {
         // Set error location to start of doc, ideally should be first token found in doc
         token.type_ = tokenError;
         token.start_ = beginDoc;
         token.end_ = endDoc;
         addError( _T("A valid JSON document must be either an array or an object value."), token );
         return false;
      }
   }

   return successful;
}

bool Reader::readValue()
{
   Token token;
   skipCommentTokens( token );
   bool successful = true;

   if( collectComments_  &&  !commentsBefore_.empty() )
   {
      currentValue().setComment( commentsBefore_, commentBefore );
      commentsBefore_ = _T("");
   }

   switch ( token.type_ )
   {
       case tokenObjectBegin:
          successful = readObject( token );
          break;
       case tokenArrayBegin:
          successful = readArray( token );
          break;
       case tokenNumber:
          successful = decodeNumber( token );
          break;
       case tokenString:
          successful = decodeString( token );
          break;
       case tokenTrue:
          currentValue() = true;
          break;
       case tokenFalse:
          currentValue() = false;
          break;
       case tokenNull:
          currentValue() = Value();
          break;
       default:
          return addError( _T("Syntax error: value, object or array expected."), token );
   }

   if ( collectComments_ )
   {
      lastValueEnd_ = current_;
      lastValue_ = &currentValue();
   }

   return successful;
}

void Reader::skipCommentTokens( Token &token )
{
   if ( features_.allowComments_ )
   {
      do
      {
         readToken( token );
      }
      while ( token.type_ == tokenComment );
   }
   else
   {
      readToken( token );
   }
}

bool Reader::expectToken( TokenType type, Token &token, const TCHAR *message )
{
   readToken( token );

   if ( token.type_ != type )
   {
      return addError( message, token );
   }

   return true;
}

bool Reader::readToken( Token &token )
{
   skipSpaces();
   token.start_ = current_;
   Char c = getNextChar();
   bool ok = true;

   switch ( c )
   {
   case _T('{'):
      token.type_ = tokenObjectBegin;
      break;

   case _T('}'):
      token.type_ = tokenObjectEnd;
      break;

   case _T('['):
      token.type_ = tokenArrayBegin;
      break;

   case _T(']'):
      token.type_ = tokenArrayEnd;
      break;

   case _T('"'):
      token.type_ = tokenString;
      ok = readString();
      break;

   case _T('/'):
      token.type_ = tokenComment;
      ok = readComment();
      break;

   case _T('0'):
   case _T('1'):
   case _T('2'):
   case _T('3'):
   case _T('4'):
   case _T('5'):
   case _T('6'):
   case _T('7'):
   case _T('8'):
   case _T('9'):
   case _T('-'):
      token.type_ = tokenNumber;
      readNumber();
      break;

   case _T('t'):
      token.type_ = tokenTrue;
      ok = match( _T("rue"), 3 );
      break;

   case _T('f'):
      token.type_ = tokenFalse;
      ok = match( _T("alse"), 4 );
      break;

   case _T('n'):
      token.type_ = tokenNull;
      ok = match( _T("ull"), 3 );
      break;

   case _T(','):
      token.type_ = tokenArraySeparator;
      break;

   case _T(':'):
      token.type_ = tokenMemberSeparator;
      break;

   case 0:
      token.type_ = tokenEndOfStream;
      break;

   default:
      ok = false;
      break;
   }

   if ( !ok )
   {
      token.type_ = tokenError;
   }

   token.end_ = current_;
   return true;
}

void Reader::skipSpaces()
{
   while ( current_ != end_ )
   {
      Char c = *current_;

      if ( c == _T(' ')  || c == _T('\t') || c == _T('\r') || c == _T('\n') )
      {
         ++current_;
      }
      else
      {
         break;
      }
   }
}

bool Reader::match( Location pattern, int patternLength )
{
   if ( end_ - current_ < patternLength )
   {
      return false;
   }

   int index = patternLength;

   while ( index-- )
   {
      if ( current_[index] != pattern[index] )
      {
         return false;
      }
   }

   current_ += patternLength;
   return true;
}

bool Reader::readComment()
{
   Location commentBegin = current_ - 1;
   Char c = getNextChar();
   bool successful = false;

   if( c == _T('*') )
   {
      successful = readCStyleComment();
   }
   else if ( c == _T('/') )
   {
      successful = readCppStyleComment();
   }

   if ( !successful )
   {
      return false;
   }

   if ( collectComments_ )
   {
      CommentPlacement placement = commentBefore;

      if ( lastValueEnd_  &&  !containsNewLine( lastValueEnd_, commentBegin ) )
      {
         if ( c != _T('*')  ||  !containsNewLine( commentBegin, current_ ) )
         {
            placement = commentAfterOnSameLine;
         }
      }

      addComment( commentBegin, current_, placement );
   }
  
   return true;
}

void Reader::addComment( Location begin, Location end, CommentPlacement placement )
{
   assert( collectComments_ );

   if ( placement == commentAfterOnSameLine )
   {
      assert( lastValue_ != 0 );
      lastValue_->setComment( _string( begin, end ), placement );
   }
   else
   {
      if ( !commentsBefore_.empty() )
      {
         commentsBefore_ += _T("\n");
      }

      commentsBefore_ += _string( begin, end );
   }
}

bool Reader::readCStyleComment()
{
   while ( current_ != end_ )
   {
      Char c = getNextChar();
      
      if( c == _T('*')  &&  *current_ == _T('/') )
      {
         break;
      }
   }

   return getNextChar() == _T('/');
}

bool Reader::readCppStyleComment()
{
   while ( current_ != end_ )
   {
      Char c = getNextChar();

      if (  c == _T('\r')  ||  c == _T('\n') )
      {
         break;
      }
   }
   
   return true;
}

void Reader::readNumber()
{
   while ( current_ != end_ )
   {
     if( !(*current_ >= _T('0')  &&  *current_ <= _T('9') )  && !in( *current_, _T('.'), _T('e'), _T('E'), _T('+'), _T('-') ) )
     {
         break;
     }

      ++current_;
   }
}

bool Reader::readString()
{
   Char c = 0;

   while ( current_ != end_ )
   {
      c = getNextChar();

      if( c == _T('\\') )
      {
         getNextChar();
      }
      else if ( c == _T('"') )
      {
         break;
      }
   }
   
   return c == _T('"');
}

bool Reader::readObject( Token &tokenStart )
{
   Token tokenName;
   _string name;
   currentValue() = Value( objectValue );

   while( readToken( tokenName ) )
   {
      bool initialTokenOk = true;

      while( tokenName.type_ == tokenComment  &&  initialTokenOk )
      {
         initialTokenOk = readToken( tokenName );
      }

      if( !initialTokenOk )
      {
         break;
      }

      if( tokenName.type_ == tokenObjectEnd  &&  name.empty() )  // empty object
      {
         return true;
      }

      if( tokenName.type_ != tokenString )
      {
         break;
      }
      
      name = _T("");

      if( !decodeString( tokenName, name ) )
      {
         return recoverFromError( tokenObjectEnd );
      }

      Token colon;

      if( !readToken( colon ) ||  colon.type_ != tokenMemberSeparator )
      {
         return addErrorAndRecover( _T("Missing ':' after object member name"), colon, tokenObjectEnd );
      }
      
      Value &value = currentValue()[ name ];
      nodes_.push( &value );
      bool ok = readValue();
      nodes_.pop();
      
      if( !ok ) // error already set
      {
         return recoverFromError( tokenObjectEnd );
      }

      Token comma;

      if( !readToken( comma ) || ( comma.type_ != tokenObjectEnd  && comma.type_ != tokenArraySeparator && comma.type_ != tokenComment ) )
      {
         return addErrorAndRecover( _T("Missing ',' or '}' in object declaration"), comma, tokenObjectEnd );
      }

      bool finalizeTokenOk = true;

      while( comma.type_ == tokenComment && finalizeTokenOk )
      {
         finalizeTokenOk = readToken( comma );
      }

      if( comma.type_ == tokenObjectEnd )
      {
         return true;
      }
   }
   
   return addErrorAndRecover( _T("Missing '}' or object member name"), tokenName, tokenObjectEnd );
}

bool Reader::readArray( Token &tokenStart )
{
   currentValue() = Value( arrayValue );
   skipSpaces();

   if( *current_ == _T(']') ) // empty array
   {
      Token endArray;
      readToken( endArray );
      return true;
   }

   int index = 0;

   while ( true )
   {
      Value &value = currentValue()[ index++ ];
      nodes_.push( &value );
      bool ok = readValue();
      nodes_.pop();

      if( !ok ) // error already set
      {
         return recoverFromError( tokenArrayEnd );
      }

      Token token;
      // Accept Comment after last item in the array.
      ok = readToken( token );

      while( token.type_ == tokenComment  &&  ok )
      {
         ok = readToken( token );
      }
      
      bool badTokenType = ( token.type_ == tokenArraySeparator && token.type_ == tokenArrayEnd );

      if( !ok  ||  badTokenType )
      {
         return addErrorAndRecover( _T("Missing ',' or ']' in array declaration"), token, tokenArrayEnd );
      }

      if( token.type_ == tokenArrayEnd )
      {
         break;
      }
   }
   
   return true;
}

bool Reader::decodeNumber( Token &token )
{
   bool isDouble = false;

   for( Location inspect = token.start_; inspect != token.end_; ++inspect )
   {
      isDouble = isDouble || in( *inspect, _T('.'), _T('e'), _T('E'), _T('+') ) || ( *inspect == _T('-') &&  inspect != token.start_ );
   }

   if( isDouble )
   {
      return decodeDouble( token );
   }

   Location current = token.start_;
   bool isNegative = *current == _T('-');

   if( isNegative )
   {
      ++current;
   }

   Value::UInt threshold = (isNegative ? Value::UInt(-Value::minInt) : Value::maxUInt) / 10;
   Value::UInt value = 0;

   while( current < token.end_ )
   {
      Char c = *current++;

      if( c < _T('0')  ||  c > _T('9') )
      {
         return addError( _T("'") + _string( token.start_, token.end_ ) + _T("' is not a number."), token );
      }

      if( value >= threshold )
      {
         return decodeDouble( token );
      }

      value = value * 10 + Value::UInt(c - _T('0') );
   }

   if( isNegative )
   {
      currentValue() = -Value::Int( value );
   }
   else if( value <= Value::UInt(Value::maxInt) )
   {
      currentValue() = Value::Int( value );
   }
   else
   {
      currentValue() = value;
   }

   return true;
}

bool Reader::decodeDouble( Token &token )
{
   double value = 0;
   const int bufferSize = 32;
   int count;
   int length = int( token.end_ - token.start_ ) * 2;

   if( length <= bufferSize )
   {
      Char buffer[bufferSize];
      memcpy( buffer, token.start_, length );
      buffer[length] = 0;
      count = _stscanf( buffer, _T("%lf"), &value );
   }
   else
   {
      _string buffer( token.start_, token.end_ );
      count = _stscanf( buffer.c_str(), _T("%lf"), &value );
   }

   if( count != 1 )
   {
      return addError( _T("'") + _string( token.start_, token.end_ ) + _T("' is not a number."), token );
   }

   currentValue() = value;
   return true;
}

bool Reader::decodeString( Token &token )
{
   _string decoded;

   if ( !decodeString( token, decoded ) )
   {
      return false;
   }

   currentValue() = decoded;
   return true;
}

bool Reader::decodeString( Token &token, _string &decoded )
{
   decoded.reserve( token.end_ - token.start_ - 2 );
   Location current = token.start_ + 1; // skip '"'
   Location end = token.end_ - 1;      // do not include '"'

   while( current != end )
   {
      Char c = *current++;

      if( c == _T('"') )
      {
         break;
      }
      else if( c == _T('\\') )
      {
         if( current == end )
         {
            return addError( _T("Empty escape sequence in string"), token, current );
         }

         Char escape = *current++;

         switch ( escape )
         {
             case _T('"'): decoded += _T('"'); break;
             case _T('/'): decoded += _T('/'); break;
             case _T('\\'): decoded += _T('\\'); break;
             case _T('b'): decoded += _T('\b'); break;
             case _T('f'): decoded += _T('\f'); break;
             case _T('n'): decoded += _T('\n'); break;
             case _T('r'): decoded += _T('\r'); break;
             case _T('t'): decoded += _T('\t'); break;
             case _T('u'):
                {
                   unsigned int unicode;

                   if ( !decodeUnicodeCodePoint( token, current, end, unicode ) )
                   {
                      return false;
                   }

                   decoded += codePointToUTF8(unicode);
                }
                break;

             default:
                return addError( _T("Bad escape sequence in string"), token, current );
         }
      }
      else
      {
         decoded += c;
      }
   }
   
   return true;
}

bool Reader::decodeUnicodeCodePoint( Token &token, Location &current, Location end, unsigned int &unicode )
{
   if( !decodeUnicodeEscapeSequence( token, current, end, unicode ) )
   {
      return false;
   }

   if( unicode >= 0xD800 && unicode <= 0xDBFF )
   {
      // surrogate pairs
      if( end - current < 6 )
      {
         return addError( _T("additional six characters expected to parse unicode surrogate pair."), token, current );
      }

      unsigned int surrogatePair;

      if( *( current++ ) == _T('\\') && *( current++ ) == _T('u') )
      {
         if( decodeUnicodeEscapeSequence( token, current, end, surrogatePair ) )
         {
            unicode = 0x10000 + ( ( unicode & 0x3FF ) << 10 ) + ( surrogatePair & 0x3FF );
         } 
         else
         {
            return false;
         }
      } 
      else
      {
         return addError( _T("expecting another \\u token to begin the second half of a unicode surrogate pair"), token, current );
      }
   }
   
   return true;
}

bool Reader::decodeUnicodeEscapeSequence( Token &token, Location &current, Location end, unsigned int &unicode )
{
   if( end - current < 4 )
   {
      return addError( _T("Bad unicode escape sequence in string: four digits expected."), token, current );
   }

   unicode = 0;

   for( int index =0; index < 4; ++index )
   {
      Char c = *current++;
      unicode *= 16;

      if( c >= _T('0')  &&  c <= _T('9') )
      {
         unicode += c - _T('0');
      }
      else if( c >= _T('a') &&  c <= _T('f') )
      {
         unicode += c - _T('a') + 10;
      }
      else if( c >= _T('A') &&  c <= _T('F') )
      {
         unicode += c - _T('A') + 10;
      }
      else
      {
         return addError( _T("Bad unicode escape sequence in string: hexadecimal digit expected."), token, current );
      }
   }
   
   return true;
}

bool Reader::addError( const _string &message, Token &token, Location extra )
{
   ErrorInfo info;
   info.token_ = token;
   info.message_ = message;
   info.extra_ = extra;
   errors_.push_back( info );
   return false;
}

bool Reader::recoverFromError( TokenType skipUntilToken )
{
   int errorCount = int(errors_.size());
   Token skip;

   while( true )
   {
      if( !readToken(skip) )
      {
         errors_.resize( errorCount ); // discard errors caused by recovery
      }

      if( skip.type_ == skipUntilToken  ||  skip.type_ == tokenEndOfStream )
      {
         break;
      }
   }
   
   errors_.resize( errorCount );
   return false;
}

bool Reader::addErrorAndRecover( const _string &message, Token &token, TokenType skipUntilToken )
{
   addError( message, token );
   return recoverFromError( skipUntilToken );
}

Value &Reader::currentValue()
{
   return *(nodes_.top());
}

Reader::Char Reader::getNextChar()
{
   if( current_ == end_ )
   {
      return 0;
   }

   return *current_++;
}

void Reader::getLocationLineAndColumn( Location location, int &line, int &column ) const
{
   Location current = begin_;
   Location lastLineStart = current;
   line = 0;

   while( current < location  &&  current != end_ )
   {
      Char c = *current++;

      if( c == _T('\r') )
      {
         if( *current == _T('\n') )
         {
            ++current;
         }

         lastLineStart = current;
         ++line;
      }
      else if( c == _T('\n') )
      {
         lastLineStart = current;
         ++line;
      }
   }
   
   // column & line start at 1
   column = int(location - lastLineStart) + 1;
   ++line;
}

_string Reader::getLocationLineAndColumn( Location location ) const
{
   int line, column;
   getLocationLineAndColumn( location, line, column );
   TCHAR buffer[18+16+16+1];
   _stprintf( buffer, _T("Line %d, Column %d"), line, column );
   return buffer;
}

_string Reader::getFormatedErrorMessages() const
{
   _string formattedMessage;

   for( Errors::const_iterator itError = errors_.begin(); itError != errors_.end(); ++itError )
   {
      const ErrorInfo &error = *itError;
      formattedMessage += _T("* ") + getLocationLineAndColumn( error.token_.start_ ) + _T("\n");
      formattedMessage += _T("  ") + error.message_ + _T("\n");

      if ( error.extra_ )
      {
         formattedMessage += _T("See ") + getLocationLineAndColumn( error.extra_ ) + _T(" for detail.\n");
      }
   }
   
   return formattedMessage;
}

std::_tistream& operator>>( std::_tistream &sin, Value &root )
{
    Json::Reader reader;
    bool ok = reader.parse(sin, root, true);
    
    //JSON_ASSERT( ok );
    if( !ok )
    {
        USES_CONVERSION;
        std::string str = W2A( reader.getFormatedErrorMessages().c_str() );
        throw std::runtime_error( str );
    }

    return sin;
}


} // namespace Json
