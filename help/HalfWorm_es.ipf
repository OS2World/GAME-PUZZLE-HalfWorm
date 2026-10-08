.* HalfWorm help - es
:userdoc.

:h1 res=2100.Acerca de HalfWorm
:i1.Acerca de HalfWorm
:p.
HalfWorm es un juego de gusanos para dos jugadores en OS/2 y ArcaOS, escrito en 1997 por Jan M. Danielsson. Esta version es una compilacion con Open Watcom, con menus y ayuda en seis idiomas, atajos de teclado y opciones guardadas.
:p.
Dos gusanos se mueven por un tablero, crecen comiendo manzanas y se disparan entre si. Gana el ultimo gusano que quede vivo.
:p.
Mas informacion&colon.
:p.
:link reftype=hd res=2101.Como jugar:elink.
.br
:link reftype=hd res=2102.Controles:elink.
.br
:link reftype=hd res=2103.Manzanas y mejoras:elink.
.br
:link reftype=hd res=2104.Menu Juego:elink.
.br
:link reftype=hd res=2105.Menu Opciones:elink.
.br
:link reftype=hd res=2106.Menu Ayuda:elink.
.br
:link reftype=hd res=2107.Copyright:elink.


:h1 res=2101.Como jugar
:i1.Como jugar
:ul compact.
:li.Elija Nuevo Juego en el menu Juego (Ctrl+N o F2) para empezar una ronda. Los dos gusanos empiezan a la vez.
:li.Cada jugador gira su gusano a izquierda o derecha y dispara misiles. El gusano siempre avanza.
:li.El tablero es continuo&colon. un gusano que sale por un borde vuelve a entrar por el borde opuesto.
:li.Coma manzanas para crecer y obtener habilidades especiales. Algunas manzanas ayudan y otras le hacen un blanco mas facil. Vea Manzanas y mejoras.
:li.Un gusano muere si choca con un gusano (el mismo o el otro) o si le alcanza un misil. Gana la ronda el ultimo gusano vivo; si mueren a la vez es un empate. La barra de estado muestra el resultado de la partida anterior.
:li.Salir Juego (Ctrl+Q) termina la ronda actual y empieza otra. Pausar Juego es Ctrl+P.
:eul.

:h1 res=2102.Controles
:i1.Controles
:p.
Cada jugador usa tres teclas. Abajo se muestran las predeterminadas; se cambian con Opciones - Teclas. Las teclas se leen directamente del teclado, asi que se pueden mantener varias a la vez.
:p.
:dl break=all.
:dt.Jugador 1 - A
:dd.Girar a la izquierda (sentido antihorario).
:dt.Jugador 1 - D
:dd.Girar a la derecha (sentido horario).
:dt.Jugador 1 - W
:dd.Disparar.
:dt.Jugador 2 - Flecha izquierda
:dd.Girar a la izquierda.
:dt.Jugador 2 - Flecha derecha
:dd.Girar a la derecha.
:dt.Jugador 2 - Flecha arriba
:dd.Disparar.
:dt.Ctrl+N o F2
:dd.Nuevo juego.
:dt.Ctrl+P
:dd.Pausar / continuar.
:dt.Ctrl+Q
:dd.Terminar la ronda actual.
:dt.Ctrl+X o F3
:dd.Salir de HalfWorm.
:dt.Ctrl+D
:dd.Ventana doble activada / desactivada.
:dt.Ctrl+B
:dd.Fondo Activo activado / desactivado.
:dt.Ctrl+F
:dd.Controles Marco - oculta / muestra la barra de titulo y el menu.
:dt.F1
:dd.Ayuda.
:edl.

:h1 res=2103.Manzanas y mejoras
:i1.Manzanas y mejoras
:p.
De vez en cuando aparecen manzanas en el tablero. El efecto de cada una es una sorpresa y se queda con el gusano; el mensaje de la barra de estado le dice lo que ha pasado.
:p.
:dl break=all.
:dt.Velocidad
:dd.El gusano se vuelve mas lento o mas rapido.
:dt.Cadencia de disparo
:dd.Puede disparar con menos o con mas frecuencia.
:dt.Velocidad de misiles
:dd.Sus misiles viajan mas lentos o mas rapidos.
:dt.Lanzador
:dd.Obtiene un lanzador mejorado (mas misiles a la vez) o pierde uno.
:dt.Explosiones
:dd.Las explosiones de los misiles son mayores o menores.
:dt.Rebotes
:dd.Los misiles rebotan en los bordes mas o menos veces.
:dt.Misiles guiados
:dd.Los misiles se dirigen hacia el otro gusano.
:dt.Misiles que atraviesan
:dd.Los misiles pasan de un borde al opuesto como los gusanos.
:dt.Misiles borrachos
:dd.Los misiles se desvian de forma impredecible.
:dt.Disparo automatico
:dd.El gusano dispara solo sin parar.
:dt.Pacifista
:dd.No puede disparar hasta que el efecto se cancele.
:dt.Contoneo
:dd.El gusano se retuerce y es mas dificil de dirigir.
:dt.Atraccion fatal
:dd.El gusano es atraido hacia su enemigo.
:dt.Reinicio
:dd.Se borran todos los efectos y el gusano queda como nuevo.
:dt.Infestada
:dd.Una manzana llena de gusanos - no tiene efecto.
:edl.

:h1 res=2104.Menu Juego
:i1.Menu Juego
:dl break=all.
:dt.Nuevo Juego (Ctrl+N, F2)
:dd.Empieza una nueva ronda.
:dt.Pausar Juego (Ctrl+P)
:dd.Detiene el juego hasta que se elija otra vez.
:dt.Salir Juego (Ctrl+Q)
:dd.Termina la ronda actual y empieza otra.
:dt.Salir (Ctrl+X, F3)
:dd.Cierra HalfWorm. Las opciones se guardan si Guardar al salir esta activado.
:edl.

:h1 res=2105.Menu Opciones
:i1.Menu Opciones
:dl break=all.
:dt.Tamano tablero...
:dd.Fija el ancho y el alto del tablero en pixeles. Maximizar para el escritorio y Maximizar para doble tamano ajustan el tablero a la pantalla.
:dt.Teclas...
:dd.Abre el dialogo de asignacion de teclas de los dos jugadores.
:dt.Doble ventana (Ctrl+D)
:dd.Muestra el juego al doble de tamano.
:dt.Efectos de sonido
:dd.Activado enciende o apaga el sonido; Volumen fija el nivel.
:dt.Idioma
:dd.Cambia el idioma de los menus y de esta ayuda. Disponibles&colon. English, Espanol, Nederlands, Deutsch, Francais, Italiano.
:dt.Fondo Activo (Ctrl+B)
:dd.El juego sigue funcionando mientras otra ventana esta activa.
:dt.Controles Marco (Ctrl+F)
:dd.Oculta o muestra la barra de titulo y el menu. Pulse Ctrl+F otra vez para recuperarlos.
:dt.Guardar al salir
:dd.Guarda las opciones, las teclas y el idioma en HalfWorm.ini al salir. Activado de forma predeterminada.
:edl.

:h1 res=2106.Menu Ayuda
:i1.Menu Ayuda
:dl break=all.
:dt.Indice de ayuda
:dd.Muestra el indice de esta ayuda.
:dt.Ayuda general
:dd.Muestra la introduccion.
:dt.Usar la ayuda
:dd.Explica como usar la ventana de ayuda.
:dt.Acerca de...
:dd.Muestra la version y el copyright. F1 muestra la ayuda del elemento de menu seleccionado.
:edl.

:h1 res=2107.Copyright
:i1.Copyright
:p.
HalfWorm para OS/2, version 0.9
:p.
Copyright (C) 1997 Jan M. Danielsson. Compilacion con Open Watcom y ampliaciones, 2026, OS2World.
:p.
Publicado como codigo abierto bajo la licencia BSD de 3 clausulas. Este programa se ofrece sin garantia alguna.
:p.
Los efectos de sonido y la interfaz de video DIVE requieren MMPM/2 y un controlador de pantalla compatible con DIVE.

:h1 res=2108.Tamano del tablero
:i1.Tamano del tablero
:p.
Fija el tamano del tablero de juego en pixeles. Ancho y Alto son los dos valores. Maximizar para el escritorio hace el tablero tan grande como permita la pantalla; Maximizar para doble tamano hace lo mismo cuando la ventana se muestra al doble de tamano. Aceptar aplica el nuevo tamano y Cancelar conserva el anterior. Un tamano nuevo empieza una ronda nueva.

:h1 res=2109.Asignacion de teclas
:i1.Asignacion de teclas
:p.
Elija las tres teclas de cada jugador&colon. giro antihorario (CCW), giro horario (CW) y disparo. Pulse un boton y luego la tecla que desea usar. Predeterminadas restaura las teclas estandar. Aceptar guarda las teclas y Cancelar descarta los cambios.

:h1 res=2110.Volumen de los efectos
:i1.Volumen de los efectos
:p.
Fija el volumen de los efectos de sonido de 0 a 100. Aceptar aplica el nuevo volumen y Cancelar conserva el anterior.

:euserdoc.
