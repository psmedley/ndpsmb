#include <nversion.h>
                      Samba Client Plugin fr <$OS>
                      Version <$NDPSMB_VERSION>
                      Build <$NDPSMB_BUILD>
                      __________________________________

INHALT
______

 1. Einfhrung
 2. Lizenz
 3. Installation
 3.1 <$ND>
 3.2 EVFS
 3.3 Deinstallation
 4. NDPSMB.DLL Verwendung
 4.1 <$ND>
 4.2 EVFS
 5. Fehlersuche
 6. Entwicklung


1. Einfhrung
_____________

Samba ist ein freie Implementierung des CIFS/SMB-Protokolls, mit der leistungs-
f„hige Datei- und Druckserver realisiert werden k”nnen. Ursprnglich stammt 
Samba aus der UNIX-Welt, um mit entsprechenden Windows-System kooperieren zu
k”nnen. <$OS> hat eine eigene Implementierung des CIFS/SMB-Protokolls,
n„mlich den IBM LAN Requester als Client bzw. IBM Peer/IBM LANServer/WarpServer
fr eBusiness auf der Serverseite. Diese Produkte wurden nicht seit langer Zeit
nicht mehr aktualisiert und haben einige Kompatibilit„tsprobleme mit modernen
Windows CIFS/SMB-Implementierungen.

Dieses Paket ist die <$OS> Portierung des Samba-Clients als Plugin fr

- <$ND> fr <$OS> - ein bekanntes virtuelles Dateisystem fr <$OS>
oder
- EVFS - das eComStation Virtual Filesystem fr SMB.

Der <$OS> Samba-Client bietet nahtlosen Zugriff auf CIFS/SMB-Dateifreigaben 
ber Ihr Netzwerk mit TCP/IP als das zugrunde liegende Transport-Protokoll.

Die <$OS> Samba-Client besteht aus einem Modul:
   ndpsmb.dll - das <$ND> / EVFS Plugin.
   
Versionen vor Version 1.5.0 hatte ein zweites Modul, das jetzt veraltet ist:
   smbcd.exe - der Samba Client Daemon-Prozesse. Dieser ist seit Version
               1.5.0 in den Plugin integriert

2. Lizenz
__________

Samba ist frei verfgbar unter der GNU General Public License. Sie k”nnen den 
vollst„ndigen Sourcecode auf unserem SVN-Repository bekommen. Sie finden auf 
der Samba fr <$OS> Homepage weitere Informationen ber das Herunterladen 
des Quellcodes.

Der <$ND> Plugin ist auch mit vollst„ndigem Quellcode verfgbar.

Also, wenn Sie unser Plugin verwenden, bitten wir Sie um Ihre Untersttzung fr
netlabs.org. Bitte beachten Sie die Samba fr <$OS> Homepage um mehr 
darber zu erfahren, wie Sie das Projekt untersttzen k”nnen!


Links:

Samba fr <$OS> Homepage: http://samba.netlabs.org/
<$ND> Homepage: http://www.blueprintsoftwareworks.com/netdrive/
Samba-Homepage: http://www.samba.org/


3. Installation
_______________

Das Plugin ben”tigt die klibc Laufzeitbibliothek namens libc06*. dll (die 
Innotek GCC Runtime DLL) irgendwo in Ihrem LIBPATH, wenn sie nicht schon dort
ist (Sie kommt mit allen bisherigen eComStation Versionen).
Desweiteren ben”tigt das Plugin den 32 bit TCP/IP Stack. Auch dieser kommt
mit allen eComstation Versionen mit. Falls noch der 16 bit stack installiert
ist, suchen Sie bitte im Internet nach dem 32 bit stack.

3.1 <$ND>
____________

Sie mssen <$ND> 2.2.1 oder h”her installiert haben, vor der Installation
dieses Paket. Wenn Sie 64-Bit-API-Datei verwenden m”chten, installieren Sie
<$ND> Version 2.3 oder neuer. Die Installation ist halbautomatisch und
besteht aus folgendem:

  - ndpsmb.dll Installation:
      Instpl.cmd aus diesem Paket aufrufen. Es werden alle Dateien in das
      Plugin-Verzeichnis kopiert.
      
3.2 EVFS
________

  - ndpsmb.dll Installation:
      Kopieren Sie ndpsmb.dll in x:\ecs\dll (wobei x: Ihr Bootlaufwerk ist)

3.3 Deinstallation
__________________

Man kann einfach das uninstpl.cmd Skript ausfhren, um den Plugin wieder zu
deinstallieren. Das gilt sowohl fr EVFS und <$ND>.

4. NDPSMB.DLL
_____________


4.1. Verwendung mit <$ND>
____________________________

Das <$ND> Samba-Plugin ist nach den allgemeinen Regeln fr die Benutzung 
von <$ND> zu verwenden. Lesen Sie bitte die Dokumentation zu <$ND> fr 
weitere Informationen. Die CIFS/SMB-Ressourcen k”nnen entweder von der 
Befehlszeile mit nd.exe oder ber die WPS mit ndpm.exe gemountet werden.

Die <$ND> Samba Plugin Parameter sind:

   Arbeitsgruppe  - den Namen der Arbeitsgruppe mit der eine Verbindung 
                    aufgebaut werden soll.
   Server         - Der Name des Servers mit dem eine Verbindung aufgebaut
                    werden soll.
   Freigabe       - Den Namen der Server-Resource (Freigabe) mit der eine 
                    Verbindung aufgebaut werden soll.
   Benutzer       - Den Name des Benutzers, der angemeldet werden soll.
   Passwort       - Das Passwort des ausgew„hlten Benutzers.
   Master         - Den Namen "Masterbrowsers" oder "WORKGROUP", je nachdem
                    was verwendet werden soll, um die Liste aller Server einer 
                    Arbeitgruppe zu erhalten.
   Masterttype    - 0, wenn "Master" der Name des "Masterbrowsers" ist,
                    1, wenn "Master" der Name der "Master-Arbeitsgruppe" ist.
   Cache timeout  - Die Zeit in Sekunden fr die der Cache gltig ist. Wenn
                    die Zeit verstrichen ist, wird das Verzeichniss neu
                    eingelesen. Fr ein sehr grosses Verzeichniss kann es von
                    Vorteil sein, wenn die Zeit erh”ht wird. (Als Regel: pro
                    500-800 Dateien 10 Sekunden)
                    Standardwert: 10 Sekunden
   Cache listings - Die Anzahl Verzeichnisse die der Cache behalten soll.
                    Vorsicht, je h”her dieser Wert desto mehr Speicher wird
                    gebraucht.
                    Standardwert: 32 Verzeichnisse
   Easupport      - 1, wenn der gewnschte Server erweiterte Attribute 
                    untersttzt, 0, wenn er es nicht tut.
   r/w            - legt fest ob eine Ressource nur lesbar (r), oder auch
                    beschreibbar (w) sein soll.  

Wenn man genau eine Freigaben mounten m”chte, muá man sowohl den "Server"
und die "Freigabe" angeben, die anderen Parameter sind optional. Diese
Serverfreigabe wird dann mit dem gewnschten Mountpoint verbunden.

Wenn man auf alle Freigaben eines Servers zugreifen m”chte, muá man nur
den "Server", nicht jedoch die "Freigabe" Parameter festlegen, die anderen
Parameter sind optional. Die Liste der Server-Freigaben werden dann mit dem
gewnschten Mountpoint verbunden.   

M”chte man auf alle Server innerhalb einer Arbeitsgruppe zugreifen, dann muá 
man nur die "Arbeitsgruppe" nicht jedoch "Server" oder "Freigabe" festlegen, 
die anderen Parameter sind wiederum optional. Die Liste der Server innerhalb 
des angegebenen Arbeitsgruppe wird mit dem gewnschten Mountpoint verbunden.   

Wenn man Zugriff auf alle verfgbaren Arbeitsgruppen innerhalb eines Master- 
Browser-Zugriff innerhalb des Netzwerks haben m”chte, dann nur den "Master", 
und weder die "Arbeitsgruppe" noch den "Server" festlegen, die anderen
Parameter sind optional. Die Liste der Arbeitsgruppen wird mit dem Mountpoint
verbunden werden. Der "Master"-Arbeitsgruppe sollte allen bekannt sein im Netz.
Der "Master"-Server sollte der Master-Browser fr Arbeitsgruppe sein.   

Standardm„áig wird der "guest" Benutzer ohne Passwort verwendet, um Zugang zu 
den gew„hlten Resourcen zu bekommen. Falls Sie einen Zugriff verweigert Fehler 
bekommen, versuchen Sie es mit einen anderen Benutzernamen und entsprechendem 
Passwort.

4.2 Verwendung mit EVFS
_______________________

eComStation kommt mit einem GUI-Programm mit dem Namen EVFSGUI, welches noch 
mehr Features und Komfort bietet als das <$ND> GUI. EVFSGUI kann auch zusammen
mit <$ND> benutzt werden. In der Dokumentation zu eComStation finden Sie mehr 
ber die Verwendung.


5. Fehlersuche
______________

Um eine Log-Datei fhren zu lassen, erzeugen Sie bitte eine leere Datei namens 
ndpsmb.dbg in das Root-Verzeichnis des Bootlaufwerks.

Falls eine LOGFILES Umgebungsvariable existiert, wird die Log-Datei dort 
erzeugt, falls nicht im <$ND> Verzeichnis. Die Logdatei heisst log.ndpsmb fr
den <$ND> Teil und log.smbc fr den Samba Teil.


6. Entwicklungsgeschichte
_________________________

#include <changelog.txt>
