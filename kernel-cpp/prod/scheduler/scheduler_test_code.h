/*! \change JS-481 Testcode */
/**
* F�r Testzwecke ist es oft notwendig Test-Codezeilen zu aktivieren oder bestehende Codezeilen zu deaktivieren.
* Einfaches ein / -auskommentieren hat den Nachteil, dass es fehleranf�llig ist. 
* Au�erdem ist Testcode unabh�ngig vom Debug-Modus.
* Deshalb ist folgendes Vorgehen gew�hlt worden:
* Die Pr�prozessordirektive TESTCODE_ACTIVATED aktiviert/deaktiviert allen Jira spezifischen Testcode.   
* Je Jira-Eintrag kann eine eigene Pr�prozessordirektive definiert werden, um dessen Jira spezifischen Testcode zu aktivieren.
* Dieses Header File ist in den entsprechenden Sourcefiles einzubinden, wo Testcode ben�tigt wird.
* Ein Beispiel w�re supervisor_client.cxx.
*/

// #define TESTCODE_ACTIVATED

#ifdef TESTCODE_ACTIVATED
	#define TESTCODE_ACTIVATED_JS481
#endif

