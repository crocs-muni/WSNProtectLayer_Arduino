/** 
 *  Wiring component for cryptographic functions providing Crypto interface via CryptoP implementation.
 *  This component wires CryptoC to implementation from CryptoP and connects to Init interface for automatic initialization.
 *  @version   1.0
 * 	@date      2012-2014
 */
#include "ProtectLayerGlobals.h"
class CryptoC {
interfaces:
	ICrypto *i_crypto;
	
private:
	MainC 		m_mainc;   
	CryptoP 	m_cryptop; 
	CryptoRawC 	m_cryptorawc;
	KeyDistribC m_keydistribc;
	AESC 		m_aesc;
	SharedDataC m_shareddatac;
	
	//MainC.SoftwareInit -> CryptoP.Init;	//auto-initialization phase 1
	
	// //Init = CryptoP.Init; 
	//Crypto = CryptoP.Crypto;

public:
	CryptoC()
	{
		i_crypto 				= m_cryptop->i_crypto;
		m_cryptop.c_shareddata 	= m_shareddatac.i_shareddata;
		m_cryptop.c_cryptoraw 	= m_cryptorawc.i_cryptoraw;
		m_cryptop.c_keydistrib 	= m_keydistribc.i_keydistrib;
		m_cryptop.c_aes 		= m_aesc.i_aes;
	
	}

	virtual ~CryptoC()
	{

	}	
}
