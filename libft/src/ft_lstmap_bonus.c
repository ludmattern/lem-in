#include "../inc/libft.h"

t_list	*ft_lstmap(t_list *lst, void *(*f)(void *), void (*del)(void *))
{
	t_list	*new_list;
	t_list	*new_elem;
	t_list	*temp;

	if (!lst)
		return (NULL);
	new_list = NULL;
	new_elem = NULL;
	temp = NULL;
	while (lst)
	{
		temp = f(lst->content);
		new_elem = ft_lstnew(temp);
		if (!new_elem)
		{
			del(temp);
			ft_lstclear(&new_list, del);
			return (NULL);
		}
		ft_lstadd_back(&new_list, new_elem);
		lst = lst->next;
	}
	return (new_list);
}
