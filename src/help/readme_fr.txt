#include <nversion.h>
                          Client Samba pour <$OS>
                          Version <$NDPSMB_VERSION>
                          Construction <$NDPSMB_BUILD>
                          ____________________________

CONTENU
_______

 1. Introduction
 2. Licence
 3. Installation
 3.1 <$ND>
 3.2 EVFS
 3.3 D‚sinstallation
 4. Utilisation de NDPSMB.DLL
 4.1 <$ND>
 4.2 EVFS
 5. Debugage
 6. Historique


1. Introduction
_______________

Samba est un ensemble client/serveur de partage de fichiers et
d'imprimantes bas‚ sur les protocoles SMB/CIFS. Ces derniers
‚taient … l'origine con‡us pour que le monde UNIX puisse coop‚rer
avec les clients ou serveurs Windows et <$OS>. <$OS> a
une impl‚mentation native de l'ensemble client/serveur SMB/CIFS -
IBM LAN Manager et IBM Peer. Ces produits n'ont pas ‚t‚ mis …
jour depuis bien longtemps et pr‚sentent des problŠmes de
compatibilit‚ avec les impl‚mentations SMB/CIFS Windows
modernes. 

Cette distribution est un portage pour <$OS> du client Samba
sous forme de composant enfichable pour les deux produits
suivants :
 - <$ND> pour <$OS> - un systŠme de fichiers virtuel pour
   <$OS> bien connu, et
 - EVFS - le systŠme de fichiers Virtuel pour SMB de eComStation
   (eComStation Virtual Filesystem for SMB). 
Le client Samba pour <$OS> fournit un accŠs transparent aux
partages de fichiers SMB/CIFS sur votre r‚seau en utilisant TCP/IP
comme protocole de transport. 

Le client Samba pour <$OS> consiste en un seul module :
 - ndpsmb.dll - le composant enfichable pour <$ND>/EVFS lui-mˆme. 

Les versions ant‚rieures … la version 1.5.0 pr‚sentaient un second
module : "smbcd.exe". Il s'agissait du "d‚mon" du client Samba qui
traitait effectivement toutes les requˆtes de ndpsmb.dll. Il n'est
… pr‚sent plus du tout utilis‚. 


2. Licence
__________

Samba est disponible gratuitement sous licence GNU General Public.
Vous pouvez obtenir le code source complet depuis notre d‚p“t SVN.
Veuillez vous r‚f‚rer … la page web de Samba pour <$OS> pour
les d‚tails sur le t‚l‚chargement des sources.

Le composant enfichable pour <$ND> est aussi disponible avec son
code source complet.

Si vous utilisez notre composant enfichable, nous vous demandons
juste de donner votre contribution … netlabs.org.
Veuillez vous r‚f‚rer … la page web de Samba pour <$OS> pour
savoir ce que vous pouvez faire ! 


Liens vers les pages web : 

Samba pour <$OS> : http://samba.netlabs.org/
<$ND> : http://www.blueprintsoftwareworks.com/netdrive/
Samba : http://www.samba.org/


3. Installation
_______________

Le composant enfichable n‚cessite la pr‚sence dans l'un des
r‚pertoires de la variable d'environnement LIBPATH de votre
systŠme, du "runtime" klibc nomm‚ libc06*.dll, la bibliothŠque GCC
d'Innotek. 
Le fichier y est peut ˆtre d‚j…, cette bibliothŠque ‚tant en effet
fournie avec les versions r‚centes d'eComStation. 

Le composant enfichable n‚cessite en outre une pile TCP/IP 32
bits. Celle-ci est fournie en standard avec toutes les versions
d'eComStation. 
Si vous avez toujours une pile TCP/IP 16 bits install‚e, vous
pourrez trouver une distribution 32 bits de celle-ci en effectuant
une recherche sur Internet. 


3.1 <$ND>
____________

Vous devez avoir <$ND> version 2.2.1 ou ult‚rieure install‚e sur
votre systŠme avant d'installer cette distribution. Si vous voulez
utiliser l'API fichiers 64 bits, vous devrez installer la version 2.3
ou ult‚rieure de <$ND>. 

Pour installer ndpsmb.dll, ex‚cutez le script instpl.cmd de cette
distribution qui placera tous les fichiers n‚cessaires dans le
r‚pertoire plugin de <$ND>. 


3.2 EVFS
________

Pour installer de ndpsmb.dll, placez  ndpsmb.dll dans x:\ecs\dll
(o— x: correspond … la lettre de votre unit‚ d'amor‡age). 


3.3 D‚sinstallation
___________________

Ex‚cutez le script afin de d‚sinstaller le plugin.
Cela fonctionne pour les deux EVFS et <ND$>.


4. Utilisation de NDPSMB.DLL
____________________________

4.1. Utilisation avec <$ND>
______________________________

Le composant enfichable Samba pour <$ND> est utilisable selon les
rŠgles d'utilisation g‚n‚rale de <$ND>. Veuillez consulter la
documentation de <$ND> pour plus d'information … ce sujet. Les
ressources SMB/CIFS pourront ˆtre mont‚es soit en utilisant
l'utilitaire en ligne de commandes ND.EXE, ou en utilisant
l'utilitaire WPS NDPM.EXE de <$ND>. 

Les paramŠtres du composant enfichable pour <$ND> sont : 

   Groupe de travail - nom du groupe de travail auquel il faut se
                       joindre. 
   Serveur           - nom (ou adresse IP) du serveur sur lequel
                       il faut se connecter. 
   Partage           - nom de la ressource sur le serveur … laquelle
                       on d‚sire acc‚der. 
   Utilisateur       - nom de l'utilisateur sous lequel if faut
                       ouvrir une session. 
   Mot de passe      - mot de passe de l'utilisateur. 
   MaŒtre            - nom du serveur ou groupe de travail "maŒtre",
                       utilis‚ pour obtenir la liste des groupes de
                       travail lorsqu'un serveur ou un groupe de
                       travail concret n'a pas ‚t‚ indiqu‚. 
   Type de maŒtre    - 0 si "maŒtre" est le nom du serveur "maŒtre",
                       1 s'il s'agit du nom du groupe de travail
                       "maŒtre".
   D‚lai du cache    - dur‚e de validit‚ du cache en secondes. Si
                       la dur‚e est d‚pass‚e, le client effectue une
                       nouvelle lecture du r‚pertoire. 
                       Pour un r‚pertoire avec beaucoup de fichiers,
                       il peut ˆtre int‚ressant d'augmenter le d‚lai
                       (en rŠgle g‚n‚rale de 10 secondes par
                       tranches de 500 … 800 fichiers). 
                       10 secondes par d‚faut. 
   Listes en cache   - le nombre de r‚pertoires que le cache devrait
                       tenir. Gardez … l'esprit que plus cette
                       valeur est grande, plus il y aura de m‚moire
                       utilis‚e. 
                       32 listes par d‚faut.
   Support des A    - 1 pour que les Attributs tendus (Extended
                       Attributes) d'<$OS> soient pris en
                       charge, 0 sinon. 
   r/w               - D‚termine si une connexion est d'ˆtre en lecture 
                       seule (r) ou ‚criture (w). 

Pour monter le partage exact d'un serveur, il faut renseigner … la
fois les champs ® Serveur ¯ et ® Partage ¯. Les autres paramŠtres
sont optionnels. La racine du chemin vers le partage du serveur sera
mont‚e sur le point de montage. 

Pour acc‚der … tous les partages d'un serveur, seul le champ
® Serveur ¯ devra ˆtre renseign‚, pas le paramŠtre ® partage ¯. Les
autres paramŠtres sont optionnels. La liste des partages du serveur
sera mont‚e sur le point de montage. 

Pour acc‚der … tous les serveurs d'un groupe de travail, seul le
champ ® Groupe de travail ¯ doit ˆtre renseign‚, pas le paramŠtre
® Serveur ¯. Les autres paramŠtres sont optionnels. La liste des
serveurs du groupe de travail donn‚ sera mont‚e sur le le point de
montage. 

Pour acc‚der … tous les groupes de travail d'un maŒtre explorateur
d'un r‚seau, alors seul le paramŠtre ® MaŒtre ¯ doit ˆtre renseign‚,
pas les champs ® Groupe de travail ¯ ni ® serveur ¯. Les
autres paramŠtres sont optionnels. La liste des groupes de travail
sera mont‚e sur le point de montage. Le groupe de travail
® MaŒtre ¯ peut ˆtre l'un des groupes de travail connu sur le
r‚seau. Le serveur ® maŒtre ¯ doit ˆtre le maŒtre explorateur du
(des) groupe(s) de travail. 

Par d‚faut, l'utilisateur ® guest ¯ sans mot de passe sera utilis‚
pour acc‚der aux ressources s‚lectionn‚es. Si vous avez des erreurs
d'accŠs refus‚, essayez d'utiliser un autre couple (utilisateur, mot
de passe). 


4.2 Utilisation avec EVFS
_________________________

eComStation propose une interface utilisateur qui comprend plus de
fonctionnalit‚s que l'interface de <$ND>. Veuillez vous r‚f‚rer …
la documentation fournie avec eCS.


5. Debugage
___________

Pour produire un fichier de consignation, veuillez cr‚er un fichier
vide nomm‚ ndpsmb.dbg … la racine du disque d'amor‡age.

Le fichier de consignation sera plac‚ dans le chemin de la variable
d'environnement LOGFILES, si disponible. Si cette variable
d'environnement est absente, le fichier de rapport sera plac‚ dans le
r‚pertoire de <$ND>. Le fichier de consignation est appel‚
log.ndpsmb pour la partie <$ND> pour OS/2 et log.smbc pour ce qui
concerne Samba. 

6. Historique
_____________

#include <changelog.txt>


Traduction fran‡aise : Guillaume Gay <guillaume.gay@bigfoot.com>
Version : 2012-01-21
