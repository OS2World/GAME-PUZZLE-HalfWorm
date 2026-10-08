.* HalfWorm help - fr
:userdoc.

:h1 res=2100.A propos de HalfWorm
:i1.A propos de HalfWorm
:p.
HalfWorm est un jeu de vers pour deux joueurs sous OS/2 et ArcaOS, ecrit en 1997 par Jan M. Danielsson. Cette version est une compilation Open Watcom avec menus et aide en ligne en six langues, raccourcis clavier et parametres sauvegardes.
:p.
Deux vers se deplacent sur un plateau, grandissent en mangeant des pommes et se tirent dessus. Le dernier ver en vie gagne.
:p.
Plus d'informations&colon.
:p.
:link reftype=hd res=2101.Comment jouer:elink.
.br
:link reftype=hd res=2102.Commandes:elink.
.br
:link reftype=hd res=2103.Pommes et bonus:elink.
.br
:link reftype=hd res=2104.Menu Jeu:elink.
.br
:link reftype=hd res=2105.Menu Options:elink.
.br
:link reftype=hd res=2106.Menu Aide:elink.
.br
:link reftype=hd res=2107.Copyright:elink.


:h1 res=2101.Comment jouer
:i1.Comment jouer
:ul compact.
:li.Choisissez Nouveau Jeu dans le menu Jeu (Ctrl+N ou F2) pour commencer une manche. Les deux vers partent en meme temps.
:li.Chaque joueur fait tourner son ver vers la gauche ou vers la droite et tire des missiles. Le ver avance toujours.
:li.Le plateau est sans bords&colon. un ver qui sort par un cote rentre par le cote oppose.
:li.Mangez des pommes pour grandir et obtenir des capacites speciales. Certaines pommes vous aident, d'autres font de vous une cible plus facile. Voir Pommes et bonus.
:li.Un ver meurt s'il heurte un ver (lui-meme ou l'autre) ou s'il est touche par un missile. Le dernier ver en vie gagne la manche; si les deux meurent ensemble, c'est un match nul. La barre d'etat indique le resultat de la partie precedente.
:li.Quitter Jeu (Ctrl+Q) termine la manche en cours et en commence une nouvelle. Pause Jeu est Ctrl+P.
:eul.

:h1 res=2102.Commandes
:i1.Commandes
:p.
Chaque joueur utilise trois touches. Les touches par defaut sont indiquees ci-dessous; modifiez-les avec Options - Touches. Les touches sont lues directement sur le clavier, on peut donc en maintenir plusieurs en meme temps.
:p.
:dl break=all.
:dt.Joueur 1 - A
:dd.Tourner a gauche (sens antihoraire).
:dt.Joueur 1 - D
:dd.Tourner a droite (sens horaire).
:dt.Joueur 1 - W
:dd.Tirer.
:dt.Joueur 2 - Fleche gauche
:dd.Tourner a gauche.
:dt.Joueur 2 - Fleche droite
:dd.Tourner a droite.
:dt.Joueur 2 - Fleche haut
:dd.Tirer.
:dt.Ctrl+N ou F2
:dd.Nouveau jeu.
:dt.Ctrl+P
:dd.Pause / reprise.
:dt.Ctrl+Q
:dd.Quitter la manche en cours.
:dt.Ctrl+X ou F3
:dd.Quitter HalfWorm.
:dt.Ctrl+D
:dd.Double fenetre activee / desactivee.
:dt.Ctrl+B
:dd.Arriere-plan Actif active / desactive.
:dt.Ctrl+F
:dd.Controles Cadre - masque / affiche la barre de titre et le menu.
:dt.F1
:dd.Aide.
:edl.

:h1 res=2103.Pommes et bonus
:i1.Pommes et bonus
:p.
De temps en temps, des pommes apparaissent sur le plateau. L'effet d'une pomme est une surprise et reste avec le ver; le message de la barre d'etat vous dit ce qui s'est passe.
:p.
:dl break=all.
:dt.Vitesse
:dd.Le ver devient plus lent ou plus rapide.
:dt.Cadence de tir
:dd.Vous tirez moins souvent ou plus souvent.
:dt.Vitesse des missiles
:dd.Vos missiles vont plus lentement ou plus vite.
:dt.Lanceur
:dd.Vous obtenez un lanceur ameliore (plus de missiles a la fois) ou en perdez un.
:dt.Explosions
:dd.Les explosions des missiles deviennent plus grandes ou plus petites.
:dt.Rebonds
:dd.Les missiles rebondissent plus ou moins souvent sur les bords.
:dt.Missiles a tete chercheuse
:dd.Les missiles se dirigent vers l'autre ver.
:dt.Missiles traversants
:dd.Les missiles passent d'un bord a l'autre comme les vers.
:dt.Missiles ivres
:dd.Les missiles zigzaguent de facon imprevisible.
:dt.Tir automatique
:dd.Le ver tire tout seul sans arret.
:dt.Pacifiste
:dd.Vous ne pouvez plus tirer tant que l'effet n'est pas annule.
:dt.Ondulation
:dd.Le ver ondule et devient plus difficile a diriger.
:dt.Attraction fatale
:dd.Le ver est attire vers son adversaire.
:dt.Reinitialisation
:dd.Tous les effets disparaissent et le ver est comme neuf.
:dt.Infestee
:dd.Une pomme pleine de vers - sans effet.
:edl.

:h1 res=2104.Menu Jeu
:i1.Menu Jeu
:dl break=all.
:dt.Nouveau Jeu (Ctrl+N, F2)
:dd.Commence une nouvelle manche.
:dt.Pause Jeu (Ctrl+P)
:dd.Arrete le jeu jusqu'a ce que vous le choisissiez de nouveau.
:dt.Quitter Jeu (Ctrl+Q)
:dd.Termine la manche en cours et en commence une nouvelle.
:dt.Quitter (Ctrl+X, F3)
:dd.Ferme HalfWorm. Les parametres sont sauvegardes si Sauver a la fermeture est active.
:edl.

:h1 res=2105.Menu Options
:i1.Menu Options
:dl break=all.
:dt.Taille plateau...
:dd.Definit la largeur et la hauteur du plateau en pixels. Maximiser pour le bureau et Maximiser pour double taille ajustent le plateau a l'ecran.
:dt.Touches...
:dd.Ouvre la boite de dialogue d'affectation des touches des deux joueurs.
:dt.Double fenetre (Ctrl+D)
:dd.Affiche le jeu en double taille.
:dt.Effets sonores
:dd.Active met le son en marche ou l'arrete; Volume regle le niveau.
:dt.Langue
:dd.Change la langue des menus et de cette aide. Disponibles&colon. English, Espanol, Nederlands, Deutsch, Francais, Italiano.
:dt.Arriere-plan Actif (Ctrl+B)
:dd.Le jeu continue de tourner pendant qu'une autre fenetre est active.
:dt.Controles Cadre (Ctrl+F)
:dd.Masque ou affiche la barre de titre et le menu. Appuyez de nouveau sur Ctrl+F pour les retrouver.
:dt.Sauver a la fermeture
:dd.Sauvegarde les options, les touches et la langue dans HalfWorm.ini a la fermeture. Active par defaut.
:edl.

:h1 res=2106.Menu Aide
:i1.Menu Aide
:dl break=all.
:dt.Index de l'aide
:dd.Affiche l'index de cette aide.
:dt.Aide generale
:dd.Affiche l'introduction.
:dt.Utiliser l'aide
:dd.Explique comment utiliser la fenetre d'aide.
:dt.A propos...
:dd.Affiche la version et le copyright. F1 affiche l'aide de l'element de menu selectionne.
:edl.

:h1 res=2107.Copyright
:i1.Copyright
:p.
HalfWorm pour OS/2, version 0.9
:p.
Copyright (C) 1997 Jan M. Danielsson. Compilation Open Watcom et extensions, 2026, OS2World.
:p.
Publie en logiciel libre sous la licence BSD a 3 clauses. Ce logiciel est fourni sans aucune garantie.
:p.
Les effets sonores et l'interface video DIVE necessitent MMPM/2 et un pilote d'affichage compatible DIVE.

:h1 res=2108.Taille du plateau
:i1.Taille du plateau
:p.
Definit la taille du plateau de jeu en pixels. Largeur et Hauteur sont les deux valeurs. Maximiser pour le bureau donne au plateau la taille maximale permise par l'ecran; Maximiser pour double taille fait de meme quand la fenetre est affichee en double taille. OK applique la nouvelle taille, Annuler conserve l'ancienne. Une nouvelle taille commence une nouvelle manche.

:h1 res=2109.Affectation des touches
:i1.Affectation des touches
:p.
Choisissez les trois touches de chaque joueur&colon. rotation antihoraire (CCW), rotation horaire (CW) et tir. Cliquez sur un bouton puis appuyez sur la touche voulue. Defaut retablit les touches standard. OK enregistre les touches, Annuler abandonne les modifications.

:h1 res=2110.Volume des effets sonores
:i1.Volume des effets sonores
:p.
Regle le volume des effets sonores de 0 a 100. OK applique le nouveau volume, Annuler conserve l'ancien.

:euserdoc.
