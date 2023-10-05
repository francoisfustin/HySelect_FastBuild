#include <stdafx.h>

#include "Global.h"
#include <writer.h>
#include <utility>
#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <iostream>
#include <sstream>
#include <iomanip>

#if _MSC_VER >= 1400 // VC++ 8.0
#pragma warning( disable : 4996 )   // disable warning about strdup being deprecated.
#endif

namespace Json {

static bool isControlCharacter( TCHAR ch )
{
    return ch > 0 && ch <= 0x1F;
}

static bool containsControlCharacter( const TCHAR *ptcStr )
{
    while ( *ptcStr ) 
    {
        if ( isControlCharacter( *(ptcStr++) ) )
        {
            return true;
        }
    }

    return false;
}

static void uintToString( unsigned int uiValue, TCHAR *&ptcCurrent )
{
    *--ptcCurrent = 0;

    do
    {
        *--ptcCurrent = (uiValue % 10) + _T('0');
        uiValue /= 10;
    }
    
    while ( uiValue != 0 );
}

_string valueToString( Int iValue )
{
   TCHAR tcBuffer[32];
   TCHAR *ptcCurrent = tcBuffer + _countof( tcBuffer ); 
   bool bIsNegative = iValue < 0;

   if( true == bIsNegative )
   {
      iValue = -iValue;
   }

   uintToString( UInt(iValue), ptcCurrent );

   if( true == bIsNegative )
   {
      *--ptcCurrent = _T('-');
   }
   
   assert( ptcCurrent >= tcBuffer );
   return ptcCurrent;
}

_string valueToString( UInt uiValue )
{
   TCHAR tcBuffer[32];
   TCHAR *ptcCurrent = tcBuffer + _countof( tcBuffer );
   uintToString( uiValue, ptcCurrent );
   assert( ptcCurrent >= tcBuffer );
   return ptcCurrent;
}

_string valueToString( double dValue )
{
    TCHAR tcBuffer[32];

#if defined( _MSC_VER ) && defined( __STDC_SECURE_LIB__ ) // Use secure version with visual studio 2005 to avoid warning. 
    _stprintf_s( tcBuffer, _countof( tcBuffer ), _T("%#.16g"), dValue); 
#else	
    _stprintf( tcBuffer, _T("%#.16g"), uiValue ); 
#endif
   
    TCHAR *ptcCh = tcBuffer + _tcslen( tcBuffer ) - 1;
   
    if( *ptcCh != _T('0') )
    {
        return tcBuffer; // nothing to truncate, so save time
    }
   
    while( ptcCh > tcBuffer && *ptcCh == _T('0') )
    {
        --ptcCh;
    }

    TCHAR *ptcLast_nonzero = ptcCh;

    while( ptcCh >= tcBuffer )
    {
        switch( *ptcCh )
        {
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
                --ptcCh;
                continue;

            case _T('.'):
                // Truncate zeroes to save bytes in output, but keep one.
                *(ptcLast_nonzero + 2) = '\0';
                return tcBuffer;

            default:
                return tcBuffer;
        }
    }
   
    return tcBuffer;
}

_string valueToString( bool value )
{
    return value ? _T("true") : _T("false");
}

_string valueToQuotedString( const TCHAR *ptcValue )
{
    // Not sure how to handle unicode...
    if( _tcspbrk( ptcValue, _T("\"\\\b\f\n\r\t") ) == NULL && false == containsControlCharacter( ptcValue ) )
    {
        return _string( _T("\"") ) + ptcValue + _T("\"");
    }

    // We have to walk uiValue and escape any special characters.
    // Appending to std::string is not efficient, but this should be rare.
    // (Note: forward slashes are *not* rare, but I am not escaping them.)
    unsigned maxsize = _tcslen( ptcValue ) * 2 + 3; // allescaped+quotes+NULL

    _string result;
    result.reserve( maxsize ); // to avoid lots of mallocs
    result += _T("\"");

   for( const TCHAR *ptcC = ptcValue; _T('\0') != *ptcC; ++ptcC )
   {
        switch( *ptcC )
        {
            case _T('\"'):
                result += _T("\\\"");
                break;

            case _T('\\'):
                result += _T("\\\\");
                break;

            case _T('\b'):
                result += _T("\\b");
                break;

            case _T('\f'):
                result += _T("\\f");
                break;

            case _T('\n'):
                result += _T("\\n");
                break;

            case _T('\r'):
                result += _T("\\r");
                break;

            case _T('\t'):
                result += _T("\\t");
                break;

            //case _T('/'):
                // Even though \/ is considered a legal escape in JSON, a bare
                // slash is also legal, so I see no reason to escape it.
                // (I hope I am not misunderstanding something.
                // blep notes: actually escaping \/ may be useful in javascript to avoid </ 
                // sequence.
                // Should add a flag to allow this compatibility mode and prevent this 
                // sequence from occurring.

            default:

                if( true == isControlCharacter( *ptcC ) )
                {
                    // std::_tostringstream oss;
                    std::_tostringstream oss;
                    oss << _T("\\u") << std::hex << std::uppercase << std::setfill( _T('0') ) << std::setw( 4 ) << static_cast<int>( *ptcC );
                    result += oss.str();
                }
                else
                {
                    result += *ptcC;
                }

                break;
        }
    }

    result += _T("\"");
    return result;
}

// Class Writer
// //////////////////////////////////////////////////////////////////
Writer::~Writer()
{
}

// Class FastWriter
// //////////////////////////////////////////////////////////////////

FastWriter::FastWriter()
   : yamlCompatiblityEnabled_( false )
{
}

void FastWriter::enableYAMLCompatibility()
{
   yamlCompatiblityEnabled_ = true;
}

_string FastWriter::write( const Value &root )
{
    document_ = _T("");
    writeValue( root );
    document_ += _T("\n");
    return document_;
}

void FastWriter::writeValue( const Value &value )
{
   switch ( value.type() )
   {
   case nullValue:
      document_ += _T("null");
      break;
   case intValue:
      document_ += valueToString( value.asInt() );
      break;
   case uintValue:
      document_ += valueToString( value.asUInt() );
      break;
   case realValue:
      document_ += valueToString( value.asDouble() );
      break;
   case stringValue:
      document_ += valueToQuotedString( value.asCString() );
      break;
   case booleanValue:
      document_ += valueToString( value.asBool() );
      break;
   case arrayValue:
      {
         document_ += _T("[");
         int size = value.size();

         for ( int index =0; index < size; ++index )
         {
            if ( index > 0 )
            {
               document_ += _T(",");
            }

            writeValue( value[index] );
         }

         document_ += _T("]");
      }
      break;

   case objectValue:
      {
         Value::Members members( value.getMemberNames() );
         document_ += _T("{");

         for ( Value::Members::iterator it = members.begin(); it != members.end(); ++it )
         {
            const _string &name = *it;

            if ( it != members.begin() )
            {
               document_ += _T(",");
            }

            document_ += valueToQuotedString( name.c_str() );
            document_ += yamlCompatiblityEnabled_ ? _T(": ") : _T(":");
            writeValue( value[name] );
         }

         document_ += _T("}");
      }
      break;
   }
}


// Class StyledWriter
// //////////////////////////////////////////////////////////////////

StyledWriter::StyledWriter()
   : rightMargin_( 74 )
   , indentSize_( 3 )
{
}

_string StyledWriter::write( const Value &root )
{
   document_ = _T("");
   addChildValues_ = false;
   indentString_ = _T("");
   writeCommentBeforeValue( root );
   writeValue( root );
   writeCommentAfterValueOnSameLine( root );
   document_ += _T("\n");
   return document_;
}

void StyledWriter::writeValue( const Value &value )
{
   switch ( value.type() )
   {
   case nullValue:
      pushValue( _T("null") );
      break;
   case intValue:
      pushValue( valueToString( value.asInt() ) );
      break;
   case uintValue:
      pushValue( valueToString( value.asUInt() ) );
      break;
   case realValue:
      pushValue( valueToString( value.asDouble() ) );
      break;
   case stringValue:
      pushValue( valueToQuotedString( value.asCString() ) );
      break;
   case booleanValue:
      pushValue( valueToString( value.asBool() ) );
      break;
   case arrayValue:
      writeArrayValue( value);
      break;
   case objectValue:
      {
         Value::Members members( value.getMemberNames() );

         if ( members.empty() )
         {
             pushValue( _T("{}") );
         }
         else
         {
            writeWithIndent( _T("{") );
            indent();
            Value::Members::iterator it = members.begin();

            while ( true )
            {
               const _string &name = *it;
               const Value &childValue = value[name];
               writeCommentBeforeValue( childValue );
               writeWithIndent( valueToQuotedString( name.c_str() ) );
               document_ += _T(" : ");
               writeValue( childValue );

               if ( ++it == members.end() )
               {
                  writeCommentAfterValueOnSameLine( childValue );
                  break;
               }
               
               document_ += _T(",");
               writeCommentAfterValueOnSameLine( childValue );
            }

            unindent();
            writeWithIndent( _T("}") );
         }
      }
      break;
   }
}

void StyledWriter::writeArrayValue( const Value &value )
{
   unsigned size = value.size();

   if ( size == 0 )
   {
      pushValue( _T("[]") );
   }
   else
   {
      bool isArrayMultiLine = isMultineArray( value );

      if ( isArrayMultiLine )
      {
         writeWithIndent( _T("[") );
         indent();
         bool hasChildValue = !childValues_.empty();
         unsigned index =0;

         while ( true )
         {
            const Value &childValue = value[index];
            writeCommentBeforeValue( childValue );

            if ( hasChildValue )
            {
               writeWithIndent( childValues_[index] );
            }
            else
            {
               writeIndent();
               writeValue( childValue );
            }

            if ( ++index == size )
            {
               writeCommentAfterValueOnSameLine( childValue );
               break;
            }
            document_ += _T(",");
            writeCommentAfterValueOnSameLine( childValue );
         }

         unindent();
         writeWithIndent( _T("]") );
      }
      else // output on a single line
      {
         assert( childValues_.size() == size );
         document_ += _T("[ ");

         for ( unsigned index =0; index < size; ++index )
         {
            if ( index > 0 )
            {
               document_ += _T(", ");
            }
            document_ += childValues_[index];
         }

         document_ += _T(" ]");
      }
   }
}

bool StyledWriter::isMultineArray( const Value &value )
{
   int size = value.size();
   bool isMultiLine = size*3 >= rightMargin_ ;
   childValues_.clear();

   for ( int index =0; index < size  &&  !isMultiLine; ++index )
   {
      const Value &childValue = value[index];
      isMultiLine = isMultiLine  ||
                     ( (childValue.isArray()  ||  childValue.isObject())  &&  
                        childValue.size() > 0 );
   }

   if ( !isMultiLine ) // check if line length > max line length
   {
      childValues_.reserve( size );
      addChildValues_ = true;
      int lineLength = 4 + (size-1)*2; // '[ ' + ', '*n + ' ]'
      for ( int index =0; index < size  &&  !isMultiLine; ++index )
      {
         writeValue( value[index] );
         lineLength += int( childValues_[index].length() );
         isMultiLine = isMultiLine  &&  hasCommentForValue( value[index] );
      }
      addChildValues_ = false;
      isMultiLine = isMultiLine  ||  lineLength >= rightMargin_;
   }
   return isMultiLine;
}


void StyledWriter::pushValue( const _string &value )
{
   if ( addChildValues_ )
   {
      childValues_.push_back( value );
   }
   else
   {
      document_ += value;
   }
}

void StyledWriter::writeIndent()
{
   if ( !document_.empty() )
   {
      TCHAR last = document_[document_.length()-1];

      if( last == _T(' ') )     // already indented
      {
         return;
      }

      if ( last != _T('\n') )    // Comments may add new-line
      {
         document_ += _T('\n');
      }
   }
   document_ += indentString_;
}

void StyledWriter::writeWithIndent( const _string &value )
{
   writeIndent();
   document_ += value;
}

void StyledWriter::indent()
{
   indentString_ += _string( indentSize_, _T(' ') );
}

void StyledWriter::unindent()
{
   assert( int(indentString_.size()) >= indentSize_ );
   indentString_.resize( indentString_.size() - indentSize_ );
}

void StyledWriter::writeCommentBeforeValue( const Value &root )
{
   if ( !root.hasComment( commentBefore ) )
   {
      return;
   }

   document_ += normalizeEOL( root.getComment( commentBefore ) );
   document_ += _T("\n");
}

void StyledWriter::writeCommentAfterValueOnSameLine( const Value &root )
{
   if ( root.hasComment( commentAfterOnSameLine ) )
   {
      document_ += _T(" ") + normalizeEOL( root.getComment( commentAfterOnSameLine ) );
   }

   if ( root.hasComment( commentAfter ) )
   {
      document_ += _T("\n");
      document_ += normalizeEOL( root.getComment( commentAfter ) );
      document_ += _T("\n");
   }
}

bool StyledWriter::hasCommentForValue( const Value &value )
{
   return value.hasComment( commentBefore )
          ||  value.hasComment( commentAfterOnSameLine )
          ||  value.hasComment( commentAfter );
}

_string StyledWriter::normalizeEOL( const _string &text )
{
   _string normalized;
   normalized.reserve( text.length() );
   const TCHAR *begin = text.c_str();
   const TCHAR *end = begin + text.length();
   const TCHAR *current = begin;

   while ( current != end )
   {
      TCHAR c = *current++;
      if ( c == _T('\r') ) // mac or dos EOL
      {
         if ( *current == _T('\n') ) // convert dos EOL
         {
            ++current;
         }

         normalized += _T('\n');
      }
      else // handle unix EOL & other char
      {
         normalized += c;
      }
   }
   
   return normalized;
}


// Class StyledStreamWriter
// //////////////////////////////////////////////////////////////////

StyledStreamWriter::StyledStreamWriter( _string indentation )
   : document_(NULL)
   , rightMargin_( 74 )
   , indentation_( indentation )
{
}

void StyledStreamWriter::write( std::_tostream &out, const Value &root )
{
   document_ = &out;
   addChildValues_ = false;
   indentString_ = _T("");
   writeCommentBeforeValue( root );
   writeValue( root );
   writeCommentAfterValueOnSameLine( root );
   *document_ << _T("\n");
   document_ = NULL; // Forget the stream, for safety.
}

void StyledStreamWriter::writeValue( const Value &value )
{
   switch ( value.type() )
   {
   case nullValue:
      pushValue( _T("null") );
      break;
   case intValue:
      pushValue( valueToString( value.asInt() ) );
      break;
   case uintValue:
      pushValue( valueToString( value.asUInt() ) );
      break;
   case realValue:
      pushValue( valueToString( value.asDouble() ) );
      break;
   case stringValue:
      pushValue( valueToQuotedString( value.asCString() ) );
      break;
   case booleanValue:
      pushValue( valueToString( value.asBool() ) );
      break;
   case arrayValue:
      writeArrayValue( value);
      break;
   case objectValue:
      {
         Value::Members members( value.getMemberNames() );
         if ( members.empty() )
            pushValue( _T("{}") );
         else
         {
            writeWithIndent( _T("{") );
            indent();
            Value::Members::iterator it = members.begin();
            while ( true )
            {
               const _string &name = *it;
               const Value &childValue = value[name];
               writeCommentBeforeValue( childValue );
               writeWithIndent( valueToQuotedString( name.c_str() ) );
               *document_ << _T(" : ");
               writeValue( childValue );

               if ( ++it == members.end() )
               {
                  writeCommentAfterValueOnSameLine( childValue );
                  break;
               }
               
               *document_ << _T(",");
               writeCommentAfterValueOnSameLine( childValue );
            }

            unindent();
            writeWithIndent( _T("}") );
         }
      }
      break;
   }
}

void StyledStreamWriter::writeArrayValue( const Value &value )
{
   unsigned size = value.size();

   if ( size == 0 )
   {
      pushValue( _T("[]") );
   }
   else
   {
      bool isArrayMultiLine = isMultineArray( value );

      if ( isArrayMultiLine )
      {
         writeWithIndent( _T("[") );
         indent();
         bool hasChildValue = !childValues_.empty();
         unsigned index =0;

         while ( true )
         {
            const Value &childValue = value[index];
            writeCommentBeforeValue( childValue );

            if ( hasChildValue )
            {
               writeWithIndent( childValues_[index] );
            }
            else
            {
	        writeIndent();
               writeValue( childValue );
            }
            
            if ( ++index == size )
            {
               writeCommentAfterValueOnSameLine( childValue );
               break;
            }
            
            *document_ << _T(",");
            writeCommentAfterValueOnSameLine( childValue );
         }

         unindent();
         writeWithIndent( _T("]") );
      }
      else // output on a single line
      {
         assert( childValues_.size() == size );
         *document_ << _T("[ ");

         for ( unsigned index =0; index < size; ++index )
         {
            if ( index > 0 )
            {
               *document_ << _T(", ");
            }

            *document_ << childValues_[index];
         }
         *document_ << _T(" ]");
      }
   }
}

bool StyledStreamWriter::isMultineArray( const Value &value )
{
   int size = value.size();
   bool isMultiLine = size*3 >= rightMargin_ ;
   childValues_.clear();

   for ( int index =0; index < size  &&  !isMultiLine; ++index )
   {
      const Value &childValue = value[index];
      isMultiLine = isMultiLine  ||
                     ( (childValue.isArray()  ||  childValue.isObject())  &&  
                        childValue.size() > 0 );
   }

   if ( !isMultiLine ) // check if line length > max line length
   {
      childValues_.reserve( size );
      addChildValues_ = true;
      int lineLength = 4 + (size-1)*2; // '[ ' + ', '*n + ' ]'

      for ( int index =0; index < size  &&  !isMultiLine; ++index )
      {
         writeValue( value[index] );
         lineLength += int( childValues_[index].length() );
         isMultiLine = isMultiLine  &&  hasCommentForValue( value[index] );
      }

      addChildValues_ = false;
      isMultiLine = isMultiLine  ||  lineLength >= rightMargin_;
   }
   return isMultiLine;
}

void StyledStreamWriter::pushValue( const _string &value )
{
   if ( addChildValues_ )
   {
      childValues_.push_back( value );
   }
   else
   {
      *document_ << value;
   }
}

void StyledStreamWriter::writeIndent()
{
  /*
    Some comments in this method would have been nice. ;-)

   if ( !document_.empty() )
   {
      TCHAR last = document_[document_.length()-1];

      if ( last == _T(' ') )     // already indented
      {
         return;
      }

      if ( last != _T('\n') )    // Comments may add new-line
      {
         *document_ << _T('\n');
      }
   }
  */
   *document_ << _T('\n') << indentString_;
}

void StyledStreamWriter::writeWithIndent( const _string &value )
{
   writeIndent();
   *document_ << value;
}

void StyledStreamWriter::indent()
{
   indentString_ += indentation_;
}

void StyledStreamWriter::unindent()
{
   assert( indentString_.size() >= indentation_.size() );
   indentString_.resize( indentString_.size() - indentation_.size() );
}

void StyledStreamWriter::writeCommentBeforeValue( const Value &root )
{
   if ( !root.hasComment( commentBefore ) )
   {
      return;
   }

   *document_ << normalizeEOL( root.getComment( commentBefore ) );
   *document_ << _T("\n");
}

void StyledStreamWriter::writeCommentAfterValueOnSameLine( const Value &root )
{
   if ( root.hasComment( commentAfterOnSameLine ) )
   {
      *document_ << _T(" ") + normalizeEOL( root.getComment( commentAfterOnSameLine ) );
   }

   if ( root.hasComment( commentAfter ) )
   {
      *document_ << _T("\n");
      *document_ << normalizeEOL( root.getComment( commentAfter ) );
      *document_ << _T("\n");
   }
}

bool StyledStreamWriter::hasCommentForValue( const Value &value )
{
   return value.hasComment( commentBefore )
          ||  value.hasComment( commentAfterOnSameLine )
          ||  value.hasComment( commentAfter );
}

_string StyledStreamWriter::normalizeEOL( const _string &text )
{
   _string normalized;
   normalized.reserve( text.length() );
   const TCHAR *begin = text.c_str();
   const TCHAR *end = begin + text.length();
   const TCHAR *current = begin;

   while ( current != end )
   {
      TCHAR c = *current++;

      if( c == _T('\r') ) // mac or dos EOL
      {
         if( *current == _T('\n') ) // convert dos EOL
         {
            ++current;
         }

         normalized += '\n';
      }
      else // handle unix EOL & other char
      {
         normalized += c;
      }
   }
   
   return normalized;
}

std::_tostream& operator<<( std::_tostream &sout, const Value &root )
{
   Json::StyledStreamWriter writer;
   writer.write(sout, root);
   return sout;
}


} // namespace Json
