#include "nversion.h"

:userdoc.

.nameit symbol=os text='<$OS>'
.nameit symbol=nd text='<$ND> f&ue.r &os.'
.nameit symbol=ndcp text='<$ND> f&ue.r &os. Control Panel'
.nameit symbol=mp text='mount point'
.nameit symbol=ressmb text='Samba Resource Parameter'
.nameit symbol=usage text='Benutzung'
.nameit symbol=build text='Samba Client Plugin f&ue.r &os. version <$NDPSMB_VERSION> build <$NDPSMB_BUILD>'

:title.<$ND> f&ue.r &os. Control Panel Hilfe

:docprof toc=123456.
.******************************************************
:h1.Einf&ue.hrung
:p.
Samba ist ein freie Implementierung des CIFS&slr.SMB-Protokolls&comma. mit der leistungs-
f&ae.hige Datei- und Druckserver realisiert werden k&oe.nnen. Urspr&ue.nglich stammt 
Samba aus der UNIX-Welt&comma. um mit entsprechenden Windows-System kooperieren zu
k&oe.nnen. &os. hat eine eigene Implementierung des CIFS&slr.SMB-Protokolls&comma.
n&ae.mlich den IBM LAN Requester als Client bzw. IBM Peer&slr.IBM LANServer&slr.WarpServer
f&ue.r eBusiness auf der Serverseite. Diese Produkte wurden nicht seit langer Zeit
nicht mehr aktualisiert und haben einige Kompatibilit&ae.tsprobleme mit modernen
Windows CIFS&slr.SMB-Implementierungen.

:p.
Dieses Paket ist die &os. Portierung des Samba-Clients als Plugin f&ue.r&colon.
:ul.
:li.&nd. - ein bekanntes virtuelles Dateisystem f&ue.r &os.
:li.EVFS - das eComStation Virtual Filesystem f&ue.r SMB
:eul.
:p.
Der &os. Samba-Client bietet nahtlosen Zugriff auf CIFS&slr.SMB-Dateifreigaben 
&ue.ber Ihr Netzwerk mit TCP&slr.IP als das zugrunde liegende Transport-Protokoll.
:p.
Der &os. Samba-Client besteht aus einem Modul&colon.
:ul.
:li.ndpsmb.dll - das <$ND> &slr. EVFS Plugin.
:eul.:p.
Versionen vor Version 1.5.0 hatten ein zweites Modul&comma. das jetzt veraltet ist&colon.
:ul.
:li.smbcd.exe  - der Samba-Client-Daemon-Prozesse. Dieser ist seit Version 1.5.0 in den Plugin integriert
:eul.
.******************************************************
:h1.Lizenz
:p.
Samba ist frei verf&ue.gbar unter der GNU General Public License. Sie k&oe.nnen den 
vollst&ae.ndigen Sourcecode auf unserem SVN-Repository bekommen. Sie finden auf 
der Samba f&ue.r &os. Homepage weitere Informationen &ue.ber das Herunterladen 
des Quellcodes.
:p.
Das <$ND> Plugin f&ue.r &os. ist auch mit vollst&ae.ndigem Quellcode verf&ue.gbar.
:p.
Also&comma. wenn Sie unser Plugin verwenden&comma. bitten wir Sie um Ihre Unterst&ue.tzung f&ue.r
netlabs.org. Bitte beachten Sie die Samba f&ue.r &os. Homepage um mehr 
dar&ue.ber zu erfahren&comma. wie Sie das Projekt unterst&ue.tzen k&oe.nnen!
:h2 res=003.Links
:p.
:ul.
:li.Samba f&ue.r &os. Homepage (http&colon.&slr.&slr.svn.netlabs.org&slr.samba)
:li.<$ND> Homepage (http&colon.&slr.&slr.www.blueprintsoftwareworks.com&slr.netdrive)
:li.Samba Homepage (http&colon.&slr.&slr.www.samba.org)
:eul.
.*******************************************************
:h1 res=1000 group=2 x=left y=top width=100% height=100% scroll=both.&ressmb.
:i1 id=ressmb.&ressmb.
:ul.
:li.:link reftype=hd res=100.&ressmb.:elink.
:li.:link reftype=hd res=101.&usage.:elink.
:eul.
:h2 res=100.&ressmb.
:p.
Die <$ND> Samba Plugin Parameter sind&colon.
:parml.
:pt.Arbeitsgruppe
:pd.den Namen der Arbeitsgruppe mit der eine Verbindung aufgebaut werden soll.

:pt.Server
:pd.Der Name oder IP Adresse des Servers zu dem verbunden werden soll.

:pt.Freigabe
:pd.Den Namen der Server-Resource (Freigabe) mit der eine Verbindung aufgebaut werden soll.

:pt.Benutzer
:pd.Den Name des Benutzers&comma. der angemeldet werden soll.

:pt.Passwort
:pd.Das Passwort des ausgew&ae.hlten Benutzers.

:pt.Master
:pd.Den Namen "Masterbrowsers" oder "WORKGROUP"&comma. je nachdem was verwendet werden soll&comma. um die Liste aller Server einer Arbeitgruppe zu erhalten.

:pt.Mastertype
:pd.0&comma. wenn "Master" der Name des "Masterbrowsers" ist&comma. 1&comma. wenn "Master" der Name der "Master-Arbeitsgruppe" ist.

:pt.Cache timeout
:pd.Die Zeit in Sekunden fÅr die der Cache gÅltig ist. Wenn die Zeit verstrichen ist, wird das Verzeichniss neu eingelesen. FÅr ein sehr 
grosses Verzeichniss kann es von Vorteil sein, wenn die Zeit erhîht wird (Als Regel: pro 500-800 Dateien 10 Sekunden). Standardwert: 10 Sekunden

:pt.Cache listings
:pd.Die Anzahl Verzeichnisse die der Cache behalten soll. Vorsicht, je hîher dieser Wert desto mehr Memory wird gebraucht. Standardwert: 32 Verzeichnisse

:pt.EA Support
:pd.1&comma. wenn der gew&ue.nschte Server erweiterte Attribute 
 unterst&ue.tzt&comma. 0&comma. wenn er es nicht tut.

:eparml.

.******************************************************
:h2 res=101.&usage.
:p.
Wenn man genau eine Freigaben mounten m&oe.chte&comma. mu&Beta. man sowohl den "Server" und die "Freigabe" angeben&comma. die anderen Parameter sind optional. Diese Serverfreigabe wird dann mit dem gew&ue.nschten Mountpoint verbunden.
:p.
Wenn man auf alle Freigaben eines Servers zugreifen m&oe.chte&comma. mu&Beta. man nur den "Server"&comma. nicht jedoch die "Freigabe"
Parameter festlegen&comma. die anderen Parameter sind optional. Die Liste der Server-Freigaben werden dann mit dem gew&ue.nschten Mountpoint verbunden.
:p.
M&oe.chte man auf alle Server innerhalb einer Arbeitsgruppe zugreifen&comma. dann mu&Beta. man nur die "Arbeitsgruppe"
nicht jedoch "Server" oder "Freigabe" festlegen&comma. die anderen Parameter sind wiederum
optional. Die Liste der Server innerhalb des angegebenen Arbeitsgruppe wird mit dem gew&ue.nschten Mountpoint verbunden.
:p.
Wenn man Zugriff auf alle verf&ue.gbaren Arbeitsgruppen innerhalb eines Master-Browser-Zugriff 
innerhalb des Netzwerks haben m&oe.chte&comma. dann nur den "Master"&comma. und weder die "Arbeitsgruppe" 
noch den "Server" festlegen&comma. die anderen Parameter sind optional. Die Liste der Arbeitsgruppen
wird mit dem Mountpoint verbunden werden. Der "Master"-Arbeitsgruppe sollte allen bekannt sein
im Netz. Der "Master"-Server sollte der Master-Browser f&ue.r Arbeitsgruppe sein.
:p.
Standardm&ae.&Beta.ig wird der "guest" Benutzer ohne Passwort verwendet&comma. um Zugang zu den gew&ae.hlten Resourcen zu bekommen. 
Falls Sie einen Zugriff verweigert Fehler bekommen&comma. versuchen Sie es mit einen anderen Benutzernamen und entsprechendem Passwort.
.******************************************************
:h1.Version
:p.
&build.
:h1.Fehlersuche
:p.
Um eine Log-Datei schreiben zu lassen&comma. erzeugen Sie bitte eine 
leere Datei namens ndpsmb.dbg in das Root-Verzeichnis des Bootlaufwerks.
:p.
Falls eine LOGFILES Umgebungsvariable existiert&comma. wird die Log-Datei 
dort erzeugt&comma. falls nicht im <$ND> Verzeichnis. Die Logdatei heisst 
log.ndpsmb f&ue.r den <$ND> Teil und log.smbc f&ue.r den Samba Teil.

.******************************************************
:h1.Entwicklungsgeschichte
:p.Entwicklungsgeschichte des Samba Plugin f&ue.r &nd.
:ul.
:li.:link reftype=hd res=011.Version 1.0:elink.
:li.:link reftype=hd res=012.Version 1.5:elink.
:li.:link reftype=hd res=013.Version 2.0:elink.
:li.:link reftype=hd res=014.Version 2.1:elink.
:li.:link reftype=hd res=015.Version 2.2:elink.
:li.:link reftype=hd res=016.Version 2.3:elink.
:eul.
#include <changelog.txt>
.******************************************************
:h1.Credit
:ul.
:li.Silvan Scherrer (aka diver)
:li.Herwig Bauernfeind (aka HerwigB)
:li.Vitali Pelenyov (aka sunlover)
:li.Paul Smedley (aka Smedles)
:li.Yuri Dario (aka Paperino)
:li.Alex Taylor (aka AlexT)
:li.Nikolay Kolosov (aka nickk)
:li.Ko Myun Hun (aka komh)
:li.und alle&comma. die wir vergessen haben! 
:eul.
.******************************************************

:euserdoc.
