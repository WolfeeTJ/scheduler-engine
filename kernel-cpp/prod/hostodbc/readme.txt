hostODBC
Der ODBC-Treiber f�r BS2000 und anderes


SOS Software- und Organisations-Service GmbH
Badensche Stra�e 29
D-10715 Berlin

Fax     (030) 861 33 35
e-mail  sosberlin@aol.com

-------------------------------------------------------------------------------

INSTALLATION
============

Sie installieren den Treiber durch Aufruf des Windows-Programms SETUP.EXE aus
dem Installationsverzeichnis.

Um Daten Ihres BS2000-Rechners zu erreichen, muss dort das Produkt Fileserver
der SOS GmbH installiert sein.

Die 32-Bit-Version ist NICHT auf win32s (d.h. Windows 3.1 oder 3.11) benutzbar.

F�r die 32-Bit-Version wird die Cursor-Bibliothek ODBCCR32.DLL nicht installiert.
In der Regel ist sie aber bereits vorhanden.

-------------------------------------------------------------------------------

LIZENZ
======

F�r den Betrieb ben�tigen Sie einen Lizenzschl�ssel. Sie geben ihn in der Datei
SOS.INI in Ihrem Windows-Verzeichnis ein. F�hren Sie hierzu "notepad sos.ini"
aus und suchen Sie den Abschnitt [licence]. Wenn er noch nicht eingerichtet ist,
tippen Sie folgendes ein, andernfalls erg�nzen Sie den Abschnitt entsprechend:

[licence]
key1=SOS-kunde-nr-xxxx-yyyyyyy

Ohne Lizenzschl�ssel wird der Fehler "SOS-1000" gemeldet.

Wenn Sie einen Lizenzschl�ssel zu bereits vorhandenen hinzuf�gen m�chten, dann
verwenden Sie den Eintrag mit der n�chsten freien Nummer, in diesem Fall key2=.
Die Numerierung muss l�ckenlos sein!

Die ersten drei Teile des Lizenzschl�ssel bilden die Lizenznummer SOS-kunde-nr.
Eine Lizenznummer darf an einem Tag nur an einem Computer verwendet werden. Die
Verwendung einer Lizenznummer an zwei Computern am selben Tag ist nicht zul�s-
sig.




