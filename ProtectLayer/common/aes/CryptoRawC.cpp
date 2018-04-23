/** 
 *  Wiring component for cryptographic functions providing CryptoRaw interface via CryptoRawP implementation.
 *  This component wires CryptoRawC to implementation from CryptoRawP and connects to Init interface for automatic initialization.
 *  @version   1.0
 * 	@date      2012-2014
 */
#include "ProtectLayerGlobals.h"

#include "arduino_port.h"

class CryptoRawC {

interfaces:
	ICryptoRaw    *i_cryptoraw;


private:
    MainC       m_main;
    CryptoRawP  m_cryptorawp;
    AESC        m_aesc;
	
	//MainC.SoftwareInit -> CryptoRawP.Init;	//auto-initialization
	
public:
    CryptoRawC()
    {
        i_cryptoraw = m_cryptorawp->i_cryptoraw;
        m_cryptorawp->m_aes = &m_aesc;
    
    }   
}
