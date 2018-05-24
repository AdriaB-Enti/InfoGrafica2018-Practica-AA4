#include <GL\glew.h>
#include <glm\gtc\type_ptr.hpp>
#include <glm\gtc\matrix_transform.hpp>

#include <cstdio>
#include <cassert>

#include "GL_framework.h"


/*  Para el movimiento de las cabinas de la noria: 
	Como no queremos que roten (podrian quedar mal, porque quedarían invertidas), hay que realizar calculos de movimiento
	La formula es, teniendo en cuenta que contamos con 20 cabinas 
	pos.x = R (Radio del circulo que traza) * cos(2π * frequencia(Constante que es la velocidad a la que giran) * T(El tiempo) * (2π*i(Numero de la cabina actual)/N(Total de las cabinas)));
	pos.y = R * sin(2π * frequencia * T * (2π*i/N));
*/

/*  Con Trump y la gallina, para que "roten", hay que utilizar la formula de arriba 
	Pero se tiene que implementar un pequeño cambio. Hay que meter los cubos en el mismo radio, pero con el centro un poco más arriba, porque sino se combinaran con la cabina
	
	
*/  