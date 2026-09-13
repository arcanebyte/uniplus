struct uprobe {
	char	*up_name;
	int	(*up_func)();
};

#define DELAY(n)	{ int _nn = n; while (_nn--); }

extern struct uprobe uprobe[];
