#include "lem_in.h"

bool display_input(const lem_in_parser_t *parser)
{
	if (!parser)
		return false;

	t_list *current = parser->file_content;
	while (current)
	{
		ft_putendl_fd((char *)current->content, 1);
		current = current->next;
	}

	return true;
}
