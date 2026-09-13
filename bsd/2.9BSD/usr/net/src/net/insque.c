struct vaxque {                 /* queue format expected by VAX queue instr's */
	struct vaxque *vq_next;
	struct vaxque *vq_prev;
};

/*
 * Insert an entry onto queue.
 */
_insque(e,prev)
	register struct vaxque *e,*prev;
{
	e->vq_prev = prev;
	e->vq_next = prev->vq_next;
	prev->vq_next->vq_prev = e;
	prev->vq_next = e;
}

/*
 * Remove an entry from queue.
 */
_remque(e)
	register struct vaxque *e;
{
	e->vq_prev->vq_next = e->vq_next;
	e->vq_next->vq_prev = e->vq_prev;
}
