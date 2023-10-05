#if !defined(AFX_SERIALCOM)
#define AFX_SERIALCOM

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#define XON  17
#define XOFF 19

// messages � destination de la fen�tre de traitement.
#define WM_CCOMRCV   WM_USER+100 // r�ceptions sur la voie s�rie
#define WM_CCOMEVENT WM_USER+101 // ev�nements sur la voie s�rie.
#define WM_CCOMERROR WM_USER+102 // erreurs sur la voie s�rie.

class CCom
{
public:
   CCom();
   ~CCom();

   // ouverture du port s�rie
   bool  PortOpen(int portnumber,long baudrate,char parity,int wordlength,int stopbits);
   // r�glage vitesse ,parit� ,longueur de la data et du bit de stop.
   // ex : 1 : Com1 , Parity , 8 : data 8 bits ,  de bit de stop
	//					EVENPARITY						ONESTOPBIT
	//					MARKPARITY						ONE5STOPBITS
	//					NOPARITY						TWOSTOPBITS
	//					ODDPARITY
	//					SPACEPARITY
   bool  PortSet(long baudrate,char parity,int wordlength,int stopbits);

            //  Fermeture du port.
   bool  PortClose();
   // Envoie d�une cha�ne de caract�res la longueur est facultative si elle se termine par �\0�
   bool  WriteBuffer(const char *buffer,unsigned int ucount=0);
   bool WriteChar(const char onechar);

   // lecture d�une cha�ne de caract�res d�une taille donn�e.
	   // GetCountRead() contiendra la taille reellement lue .
   int   ReadBuffer(char *buffer,unsigned int ucount);

   // lecture d'un charactere.
   bool ReadChar(char &rchar);

   // R�glage du mode de communication XON/XOFF
   bool  UseXonXoff(bool bEnable=true);

   // R�glage du mode de communication Rts/Cts
   bool  UseRtsCts(bool bEnable=true);

   // R�glage du mode de communication Dtr/Dsr
   bool  UseDtrDsr(bool bEnable=true);

   // Renvoie le nombre d'octets dans le buffer de reception avant lecture
   long SizeUsedInRXBuf();              
   long SizeUsedInTXBuf();              

   // renvoie true si le buffer de reception est vide.
   bool	IsRXEmpty();
   bool	IsTXEmpty();

   // Attente d�un �v�nement fix� par SetCommMask()
   bool  WaitCommEvent(DWORD &rEvtMask);
   // fixe la gestion des evenements sur le port serie. Voir doc MSDN :   EV_RXCHAR
   bool  SetCommMask(DWORD EvtMask);

   // recupere l'�v�nement en cours.
   DWORD GetCommMask();

   // Renvoie sous forme litterale la derniere erreur rencontr�e avec GetLastError()
   CString GetStringError(){return m_StrError;}

   // Renvoie la derniere valeur du nombre d�octets lus.
   int   GetCountRead(){return m_uiCount;}

   // fixe la fenetre parent pour les messages en provenance de la voie serie.
   void SetParentNotify(CWnd *pParent){m_pParent=pParent;}

   // purge le port serie.
   bool PurgeCom();

   // purge la r�ception
   bool	PurgeRx();

   // purge l'�mission
   bool	PurgeTx();

   // lancer le thread
   bool StartThread(CWnd *pParent);

   // redemarre le thread 
   bool	ResumeThread();

   // stop le thread.
   bool	StopThread();

   bool SetTimeouts(DWORD dwRxTimeout=5000,DWORD dwTxTimeout=5000);

   // fonction appel�e en cas d'erreur detect�e dans le thread.
   // envoie un message � la fenetre parent.
   virtual void OnError(DWORD dwError);

   HANDLE GetComHandle(){return m_hCom;};
   int GetPortNumber(){return m_iPortNumber;};
	static BOOL EnumerateComPortUsingRegistry(CStringArray& ports);


protected:

    // thread de communication.
   static UINT	Thread(LPVOID pParam);

   // r�cup�ration de l�erreur
   DWORD  GetError();

protected:
       CWnd              *m_pParent;
       CString            m_StrError;
       DWORD              m_dwComErr;
       HANDLE             m_hCom;
	   int				  m_iPortNumber;
       UINT               m_uiInputBufferSize;
       UINT               m_uiOutputBufferSize;
       DCB                m_dcb;
       DWORD              m_dwEventMask;
       COMMTIMEOUTS       m_ComTimeouts;
       OVERLAPPED         m_Ov;
       unsigned int       m_uiCount;

       long               m_lBaudRate;
       char               m_cParity;
       int                m_iWordLength;
       int                m_iStopBits;
       CWinThread*		  m_pThread;
       HANDLE			  m_hCloseCom;
       HANDLE			  m_hArrayEvent[2];
       bool               m_bThreadExist;

};
#endif // !defined(AFX_SERIALCOM)