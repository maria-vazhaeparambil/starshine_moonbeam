#include "list.h"
#include "scene.h"

/**
 * A force acting in the scene.
 */
typedef struct force force_t;

/**
 * Initializes a body wity any auxiliary info and a specific freer.
 * 
 * @param forcer applies the forces to bodies
 * @param aux auxiliary data
 * @param freer used to free the auxiliary data
 * @return a pointer to the newly allocated force
 */
force_t *force_init(force_creator_t forcer, void *aux, free_func_t freer);

/**
 * Initializes a body wity any auxiliary info and a specific freer and
 * its relevant bodies.
 * 
 * @param forcer applies the forces to bodies
 * @param aux auxiliary data
 * @param relevant_bodies the bodies impacted by the force
 * @param freer used to free the auxiliary data
 * @return a pointer to the newly allocated force
 */
force_t *force_init_with_bodies(force_creator_t forcer, void *aux,
                                list_t *relevant_bodies, free_func_t freer);

/**
 * Releases the memory allocated for a force.
 *
 * @param force a pointer to a force returned from init functions
 */
void force_free(force_t *force);

/**
 * Returns the force creator associated with the current force.
 *
 * @param force the force whose information is being retrieved
 * @return the force creator function that applies the right type of force
 */
force_creator_t get_force_creator(force_t *force);

/**
 * Returns the auxiliary data associated with the force.
 * 
 * @param force the force whose information is being retrieved
 * @return the auxiliary data of the force
 */
void *get_aux(force_t *force);

/**
 * Returns freer for the auxiliary data associated with the force.
 * 
 * @param force the force whose information is being retrieved
 * @return the freer for the auxiliary data of the force
 */
free_func_t get_freer(force_t *force);

/**
 * Returns relavant bodies that the force is applied to.
 * 
 * @param force the force whose information is being retrieved
 * @return any bodies that the force affects
 */
list_t *get_relevant_bodies(force_t *force);
