#include "libft.h"

void	ft_lstappend(t_list **alst, t_list *new)
{
	t_list	*curr;

	if (alst == NULL || new == NULL)
		return ;
	if (*alst == NULL)
	{
		*alst = new;
		return ;
	}
	curr = *alst;
	while (curr->next != NULL)
		curr = curr->next;
	curr->next = new;
}

