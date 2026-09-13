/* queue format expected by VAX queue instructions */
struct qelem {
	struct qelem	*q_forw;
	struct qelem	*q_back;
};

#define	insque(q,p)	_insque((struct qelem *) (q),(struct qelem *) (p))
#define	remque(q)	_remque((struct qelem *) (q))
