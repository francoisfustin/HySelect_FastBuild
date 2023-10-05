#if !defined(AFX_SERIALCOM)
#define AFX_SERIALCOM

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#define XON  17
#define XOFF 19

// messages à destination de la fenêtre de traitement.
#define WM_CCOMRCV   WM_USER+100 // réceptions sur la voie série
#define WM_CCOMEVENT WM_USER+101 // evénements sur la voie série.
#define WM_CCOMERROR WM_USER+102 // erreurs sur la voie série.

class CCom
{
public:
   CCom();
   ~CCom();

   // ouverture du port série
   bool  PortOpen(int portnumber,long baudrate,char parity,int wordlength,int stopbits);
   // réglage vitesse ,parité ,longueur de la data et du bit de stop.
   // ex : 1 : Com1 , Parity , 8 : data 8 bits ,  de bit de stop
	//					EVENPARITY						ONESTOPBIT
	//					MARKPARITY						ONE5STOPBITS
	//					NOPARITY						TWOSTOPBITS
	//					ODDPARITY
	//					SPACEPARITY
   bool  PortSet(long baudrate,char parity,int wordlength,int stopbits);

            //  Fermeture du port.
   bool  PortClose();
   // Envoie d’une chaîne de caractères la longueur est facultative si elle se termine par ‘\0’
   bool  WriteBuffer(const char *buffer,unsigned int ucount=0);
   bool WriteChar(const char onechar);

   // lecture d’une chaîne de caractères d’une taille donnée.
	   // GetCountRead() contiendra la taille reellement lue .
   int   ReadBuffer(char *buffer,unsigned int ucount);

   // lecture d'un charactere.
   bool ReadChar(char &rchar);

   // Réglage du mode de communication XON/XOFF
   bool  UseXonXoff(bool bEnable=true);

   // Réglage du mode de communication Rts/Cts
   bool  UseRtsCts(bool bEnable=true);

   // Réglage du mode de communication Dtr/Dsr
   bool  UseDtrDsr(bool bEnable=true);

   // Renvoie le nombre d'octets dans le buffer de reception avant lecture
   long SizeUsedInRXBuf();              
   long SizeUsedInTXBuf();              

   // renvoie true si le buffer de reception est vide.
   bool	IsRXEmpty();
   bool	IsTXEmpty();

   // Attente d’un événement fixé par SetCommMask()
   bool  WaitCommEvent(DWORD &rEvtMask);
   // fixe la gestion des evenements sur le port serie. Voir doc MSDN :   EV_RXCHAR
   bool  SetCommMask(DWORD EvtMask);

   // recupere l'événement en cours.
   DWORD GetCommMask();

   // Renvoie sous forme litterale la derniere erreur rencontrée avec GetLastError()
   CString GetStringError(){return m_StrError;}

   // Renvoie la derniere valeur du nombre d’octets lus.
   int   GetCountRead(){return m_uiCount;}

   // fixe la fenetre parent pour les messages en provenance de la voie serie.
   void SetParentNotify(CWnd *pParent){m_pParent=pParent;}

   // purge le port serie.
   bool PurgeCom();

   // purge la réception
   bool	PurgeRx();

   // purge l'émission
   bool	PurgeTx();

   // lancer le thread
   bool StartThread(CWnd *pParent);

   // redemarre le thread 
   bool	ResumeThread();

   // stop le thread.
   bool	StopThread();

   bool SetTimeouts(DWORD dwRxTimeout=5000,DWORD dwTxTimeout=5000);

   // fonction appelée en cas d'erreur detectée dans le thread.
   // envoie un message à la fenetre parent.
   virtual void OnError(DWORD dwError);

   HANDLE GetComHandle(){return m_hCom;};
   int GetPortNumber(){return m_iPortNumber;};
	static BOOL EnumerateComPortUsingRegistry(CStringArray& ports);


protected:

    // thread de communication.
   static UINT	Thread(LPVOID pParam);

   // récupération de l’erreur
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