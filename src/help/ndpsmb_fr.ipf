#include "nversion.h"

:userdoc.

.nameit symbol=os text='<$OS>'
.nameit symbol=nd text='<$ND> pour &os.'
.nameit symbol=ndcp text='Panneau de contr“le <$ND> pour &os.'
.nameit symbol=mp text='Point de montage'
.nameit symbol=ressmb text='Propri‚t‚s de la ressource Samba'
.nameit symbol=usage text='Utilisation'
.nameit symbol=build text='Client Samba pour &os. version <$NDPSMB_VERSION> construction <$NDPSMB_BUILD>'

:title.Panneau de contr“le <$ND> pour &os.
:docprof toc=123456.
.***************************************************
.* <$ND> for eComStation (OS/2) Control Panel Help - French version
.* Translator : Guillaume Gay <guillaume.gay@bigfoot.com>
.* Translation : 2011-01-11
.***************************************************

.******************************************************
.* PANNEL : INTRODUCTION
.******************************************************
:h1.Introduction
:p.
Samba est un ensemble client/serveur de partage de fichiers et
d'imprimantes bas‚ sur les protocoles SMB/CIFS. Ces derniers
‚taient … l'origine con‡us pour que le monde UNIX puisse coop‚rer
avec les clients ou serveurs Windows et &os.&per. &os. a une
impl‚mentation native de l'ensemble client/serveur SMB/CIFS -
IBM LAN Manager et IBM Peer. Ces produits n'ont pas ‚t‚ mis …
jour depuis bien longtemps et pr‚sentent des problŠmes de
compatibilit‚ avec les impl‚mentations SMB/CIFS Windows
modernes. 
:p.
Cette distribution est un portage pour &os. du client Samba
sous forme de composant enfichable pour les produits
suivants &colon.
:ul.
:li.&nd. - un systŠme de fichiers virtuel pour &os. bien
connu, et
:li.EVFS - le systŠme de fichiers Virtuel pour SMB de
eComStation (eComStation Virtual Filesystem for SMB).
:eul.
Le client Samba pour &os. fournit un accŠs transparent aux partages de
fichiers SMB/CIFS sur votre r‚seau en utilisant TCP/IP comme protocole
de transport.
:p.
Le client Samba pour &os. consiste en un seul module &colon.
:ul.
:li.ndpsmb.dll - le composant enfichable pour <$ND>/EVFS lui-mˆme.
:eul.
:p.
Les versions ant‚rieures … la version 1.5.0 pr‚sentaient un second
module &colon. "smbcd.exe". Il s'agissait du "d‚mon" du client Samba
qui traitait effectivement toutes les requˆtes de ndpsmb.dll. Il
n'est … pr‚sent plus du tout utilis‚.
:p.
.******************************************************
.* PANNEL : LICENSE
.******************************************************
:h1.Licence
:p.
Samba est disponible gratuitement sous licence GNU General Public.
Vous pouvez obtenir le code source complet depuis notre d‚p“t SVN.
Veuillez vous r‚f‚rer … la :link reftype=hd res=003.page web de
Samba pour &os.:elink. pour les d‚tails sur le t‚l‚chargement des
sources.
:p.
Le composant enfichable pour &nd. est aussi disponible avec son
code source complet. 
:p.
Si vous utilisez notre composant enfichable, nous vous demandons
juste de donner votre contribution … netlabs.org.
Veuillez vous r‚f‚rer … la :link reftype=hd res=003.page web de
Samba pour &os.:elink.  pour savoir ce que vous pouvez faire !
:p.
.******************************************************
.* PANNEL : LINKS
.******************************************************
:h2 res=003.Liens
:p.
Liens vers les pages web &colon.
:ul.
:li.Samba pour &os. &colon. http&colon.&slr.&slr.svn.netlabs.org&slr.samba
:li.<$ND> &colon. http&colon.&slr.&slr.www.blueprintsoftwareworks.com&slr.netdrive
:li.Samba &colon. http&colon.&slr.&slr.www.samba.org
:eul.
:p.
.******************************************************
.* PANNEL : SAMBA RESOURCE
.******************************************************
:h1 res=1000 group=2 x=left y=top width=100% height=100% scroll=both.&ressmb.
:i1 id=ressmb.&ressmb.
:ul.
:li.:link reftype=hd res=100.&ressmb.:elink.
:li.:link reftype=hd res=101.&usage.:elink.
:eul.
:p.
.******************************************************
.* PANNEL : SAMBA RESOURCE PROPERTIES
.******************************************************
:h2 res=100.&ressmb.
:p.
Les paramŠtres d'une ressource SMB/CIFS FS sont &colon.
:parml.
:pt.Groupe de travail
:pd.nom du groupe de travail auquel il faut se
joindre. 

:pt.Serveur
:pd.nom (ou adresse IP) du serveur sur lequel
il faut se connecter. 

:pt.Partage
:pd.nom de la ressource sur le serveur … laquelle
on d‚sire acc‚der. 

:pt.Utilisateur
:pd.nom de l'utilisateur sous lequel if faut
ouvrir une session. 

:pt.Mot de passe
:pd.mot de passe de l'utilisateur. 

:pt.MaŒtre
:pd.le nom du serveur ou groupe de travail "maŒtre", utilis‚ pour
obtenir la liste des groupes de travail lorsqu'un serveur ou un
groupe de travail concret n'a pas ‚t‚ indiqu‚. 

:pt.Type de maŒtre
:pd.0 si "maŒtre" est le nom du serveur "maŒtre",
1 s'il s'agit du nom du groupe de travail "maŒtre".

:pt.D‚lai du cache
:pd.dur‚e de validit‚ du cache en secondes. Si la dur‚e est
d‚pass‚e, le client effectue une nouvelle lecture du r‚pertoire. 
Pour un r‚pertoire avec beaucoup de fichiers, il peut ˆtre
int‚ressant d'augmenter le d‚lai (en rŠgle g‚n‚rale de 10 secondes
par tranches de 500 … 800 fichiers). 10 secondes par d‚faut. 

:pt.Listes en cache
:pd.le nombre de r‚pertoires que le cache devrait tenir. Gardez
… l'esprit que plus cette valeur est grande, plus il y aura de
m‚moire utilis‚e. 32 listes par d‚faut. 
 
:pt.Support des A
:pd.1 pour que les Attributs tendus (Extended Attributes) d'&os.
soient pris en charge, 0 sinon.

:pt.r/w
:pd.D‚termine si une connexion est d'ˆtre en lecture seule (r) ou ‚criture (w). 
 
:eparml.
:p.
.******************************************************
.* PANNEL : USAGE
.******************************************************
:h2 res=101.&usage.
:p.
Pour monter le partage exact d'un serveur, il faut renseigner … la
fois les champs &odqf.Serveur&cdqf. et &odqf.Partage&cdqf.&per. Les
autres paramŠtres sont optionnels. La racine du chemin vers le
partage du serveur sera mont‚e sur le point de montage.
:p.
Pour acc‚der … tous les partages d'un serveur, seul le champ
&odqf.Serveur&cdqf. devra ˆtre renseign‚, pas le paramŠtre
&odqf.partage&cdqf.&per. Les autres paramŠtres sont optionnels. La
liste des partages du serveur sera mont‚e sur le point de montage.
:p.
Pour acc‚der … tous les serveurs d'un groupe de travail, seul le
le champ &odqf.Groupe de travail&cdqf. doit ˆtre renseign‚, pas le
paramŠtre &odqf.Serveur&cdqf.&per. Les autres paramŠtres sont
optionnels. La liste des serveurs du groupe de travail donn‚ sera
mont‚e sur le le point de montage.
:p.
Pour acc‚der … tous les groupes de travail d'un maŒtre explorateur
d'un r‚seau, alors seul le paramŠtre &odqf.MaŒtre&cdqf. doit ˆtre
renseign‚, pas les champs &odqf.Groupe de travail&cdqf. ni
&odqf.serveur&cdqf.&per. Les autres paramŠtres sont optionnels. La
liste des groupes de travail sera mont‚e sur le point de montage.
Le groupe de travail &odqf.MaŒtre&cdqf. peut ˆtre l'un des groupes de
travail connu sur le r‚seau. Le serveur &odqf.maŒtre&cdqf. doit ˆtre
le maŒtre explorateur du (des) groupe(s) de travail.
:p.
Par d‚faut, l'utilisateur &odqf.guest&cdqf. sans mot de passe sera
utilis‚ pour acc‚der aux ressources s‚lectionn‚es. Si vous avez des
erreurs d'accŠs refus‚, essayez d'utiliser un autre couple
(utilisateur, mot de passe).
:p.
.******************************************************
.* PANNEL : VERSION
.******************************************************
:h1.Version
:p.
&build.
:p.
Traduction fran‡aise &colon. Guillaume Gay &lt.guillaume.gay@bigfoot.com&gt.
.br
Version 2011-01-10
:p.
.******************************************************
.* PANNEL : DEBUGGING
.******************************************************
:h1.Debugage
:p.
Pour produire un fichier de consignation, vous devez cr‚er un fichier
vide nomm‚ ndpsmb.dbg … la racine du disque d'amor‡age.
:p.
Le fichier de consignation sera plac‚ dans le chemin de la variable
d'environnement LOGFILES, si disponible. Si cette variable
d'environnement est absente, le fichier de rapport sera alors plac‚
dans le r‚pertoire de &nd.&per. Le fichier de consignation est appel‚
log.ndpsmb pour la partie &nd. et log.smbc pour ce qui concerne
Samba. 
:p.
.******************************************************
.* PANNEL : HISTORY
.******************************************************
:h1.Historique
:p.Historique des modifications du client Samba pour &nd.
:ul.
:li.:link reftype=hd res=011.Version 1.0:elink.
:li.:link reftype=hd res=012.Version 1.5:elink.
:li.:link reftype=hd res=013.Version 2.0:elink.
:li.:link reftype=hd res=014.Version 2.1:elink.
:li.:link reftype=hd res=015.Version 2.2:elink.
:li.:link reftype=hd res=016.Version 2.3:elink.
:eul.
#include <changelog.txt>
:p.
.******************************************************
.* PANNEL : CREDIT
.******************************************************
:h1.Tableau d'honneur
:ul.
:li.Silvan Scherrer (aka diver)
:li.Herwig Bauernfeind (aka HerwigB)
:li.Vitali Pelenyov (aka sunlover)
:li.Paul Smedley (aka Smedles)
:li.Yuri Dario (aka Paperino)
:li.Alex Taylor (aka AlexT)
:li.Nikolay Kolosov (aka nickk)
:li.Ko Myun Hun (aka komh)
:li.Guillaume Gay (aka GG)
:li.et tous ceux que nous avons pu oublier
:eul.
:p.
.******************************************************
:euserdoc.
