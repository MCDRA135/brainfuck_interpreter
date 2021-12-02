#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <errno.h>

typedef enum
{
	FALSE,
	TRUE
} bool;

/* Dynamic string implementation */

typedef char* string;

string str_new(int bufsz)
{
	string str = malloc(sizeof(char) * bufsz);
	if (!str)
		return NULL;
	return str;
}

void str_destr(string str)
{
	free(str);
}

string str_concat(const string str1, const string str2)
{
	int bufsz = strlen(str1) + strlen(str2) + 1;
	string str = str_new(bufsz);
	if (!str)
		return NULL;
	sprintf_s(str, bufsz, "%s%s", str1, str2);
	return str;
}

int str_append(string* const dest, const string src)
{
	int dest_len = strlen(*dest);
	int bufsz = dest_len + strlen(src) + 1;
	string new = realloc(*dest, bufsz * sizeof(char));
	if (!new)
		return -1;
	sprintf_s(new + dest_len, bufsz - dest_len, "%s", src);
	*dest = new;
	return 0;
}

int str_count(const string str, char c)
{
	int count = 0;
	for (unsigned int i = 0; i < strlen(str); i++)
		count += (str[i] == c);
	return count;
}

/* Readline function that allocates exactly the needed amount of space, by reading stdin by blocks of size blocksz */

string readline(int blocksz, FILE* stream)
{
	if (feof(stream))
		return NULL;
	string str_line = str_new(1);
	string input = str_new(blocksz);
	string tmp_str;
	if (!(str_line && input))
		return NULL;
	strcpy_s(str_line, 1 * sizeof(char), "\0");
	while (!(strchr(str_line, '\n') || feof(stream)))
	{
		fgets(input, blocksz, stream);
		if (str_append(&str_line, input) < 0)
		{
			str_destr(str_line);
			str_destr(input);
			return NULL;
		}
	}
	str_line[*str_line ? strlen(str_line) - 1 : 0] = 0;
	str_destr(input);
	return str_line;
}

/* STACK STRUCTURE IMPLEMENTATION */

typedef struct StackElement
{
	int value;
	struct StackElement* next;
} StackElement, *Stack;

Stack stk_new()
{
	return NULL;
}

void stk_destr(Stack stk)
{
	Stack tmp;
	while(stk != NULL)
	{
		tmp = stk;
		stk = stk->next;
		free(tmp);
	}
}

bool stk_empty(Stack stk)
{
	return (bool)(stk == NULL);
}

int stk_push(Stack* stk, int value)
{
	StackElement* new = malloc(sizeof(StackElement));
	if (!new)
		return -1;
	new->value = value;
	new->next = *stk;
	*stk = new;
	return 0;
}

int stk_pop(Stack* stk, int* dest)
{
	if (stk_empty(*stk))
		return -1;
	*dest = (*stk)->value;
	Stack new = (*stk)->next;
	free(*stk);
	*stk = new;
	return 0;
}

/* BRAINFUCK MEMORY IMPLEMENTATION */

typedef struct BFMemCell
{
	char value;
	struct BFMemCell* prev;
	struct BFMemCell* next;
} *BFMem;

BFMem mem_new()
{
	BFMem new = malloc(sizeof(struct BFMemCell));
	if (!new)
		return NULL;
	new->value = 0;
	new->prev = NULL;
	new->next = NULL;
	return new;
}

void mem_destr(BFMem mem)
{
	BFMem tmp;
	BFMem prev = mem->prev;
	BFMem next = mem->next;
	free(mem);
	while (prev != NULL)
	{
		tmp = prev;
		prev = prev->prev;
		free(tmp);
	}
	while (next != NULL)
	{
		tmp = next;
		next = next->next;
		free(tmp);
	}
}

int mem_get(BFMem mem)
{
	return mem->value;
}

void mem_set(BFMem mem, int value)
{
	mem->value = value;
}

int mem_ml(BFMem* mem)
{
	if ((*mem)->prev == NULL)
	{
		BFMem prev = mem_new();
		if (!prev)
			return -1;
		prev->next = *mem;
		(*mem)->prev = prev;
	}
	*mem = (*mem)->prev;
	return 0;
}

int mem_mr(BFMem* mem)
{
	if ((*mem)->next == NULL)
	{
		BFMem next = mem_new();
		if (!next)
			return -1;
		next->prev = *mem;
		(*mem)->next = next;
	}
	*mem = (*mem)->next;
	return 0;
}

void mem_add(BFMem mem)
{
	mem->value += 1;
}

void mem_sub(BFMem mem)
{
	mem->value -= 1;
}

/* Was supposed to be a simple debugging function, but took me several hours nonethelesss, because I suck at C */
const string mem_repr(BFMem mem)
{
	BFMem first = mem;
	while (first->prev != NULL)
		first = first->prev;
	string str = str_new(2);
	int str_len;
	string tmp_str;
	if (!str)
		return NULL;
	strcpy_s(str, sizeof(char) * 2, "[");
	char value = 0;
	int bufsz = 0;

	do 
	{
		str_len = strlen(str);
		value = first->value;
		bufsz = ceil(log10(value > 0 ? value : 1) + 3.0) + str_len + 1;
		tmp_str = realloc(str, sizeof(char) * bufsz);
		if (!tmp_str)
		{
			str_destr(str);
			return NULL;
		}
		str = tmp_str;
		first = first->next;
		if (first == NULL)
			sprintf_s(str + str_len, bufsz - str_len, "%d", value);
		else
			sprintf_s(str + str_len, bufsz - str_len, "%d, ", value);
	} while (first != NULL);
	str_len = strlen(str);
	bufsz = 2 + str_len;
	tmp_str = realloc(str, sizeof(char) * bufsz);
	if (!tmp_str)
	{
		str_destr(str);
		return NULL;
	}
	str = tmp_str;
	sprintf_s(str + str_len, bufsz - str_len, "]");
	return str;
}

/* Brainfuck evaluation function */

int evaluate_bf(string code, BFMem* mem)
{
	int i = 0;											// Main loop pointer
	int code_len = strlen(code);
	int* brace_map = calloc(code_len, sizeof(int));	// Array associating each brace pair
	if (!brace_map)
		return -1;
	int op_count = 0;
	bool printed = FALSE;
	Stack index_stack = stk_new();
	for (int k = 0; k < code_len; k++)
	{
		if (code[k] == '[')
		{
			stk_push(&index_stack, k);
		}
		else if (code[k] == ']')
		{
			if (stk_empty(index_stack))
			{
				free(brace_map);
				return -1;
			}
			int v;
			stk_pop(&index_stack, &v);
			brace_map[k] = v;
			brace_map[v] = k;
		}
	}
	if (!stk_empty(index_stack))
	{
		free(brace_map);
		stk_destr(index_stack);
		return -1;
	}
	stk_destr(index_stack);

	/* Main interpreter loop */
	while (i < code_len)
	{
		op_count++;
		switch (code[i])
		{
		case '<':
			if (mem_ml(mem) == -1)
				return -1;
			break;
		case '>':
			if (mem_mr(mem) == -1)
				return -1;
			break;
		case '+':
			mem_add(*mem);
			break;
		case '-':
			mem_sub(*mem);
			break;
		case '.':
			putchar(mem_get(*mem));
			printed = TRUE;
			break;
		case ',':
			mem_set(*mem, getchar());
			str_destr(readline(16, stdin));
			break;
		case '[':
			if (!mem_get(*mem))
				i = brace_map[i];
			break;
		case ']':
			if (mem_get(*mem))
				i = brace_map[i];
			break;
		default:
			op_count--;
			break;
		}
		i++;
	}
	free(brace_map);
	if (printed)
		putchar('\n');
	return op_count;
}

int code_brackets_count(string code)
{
	int k = 0;
	for (int i = 0; i < strlen(code); i++)
		k += (code[i] == '[') - (code[i] == ']');
	return k;
}

int main(int argc, char** argv)
{
	bool interactive = (argc < 2);
	if (interactive)
	{
		BFMem mem = mem_new();
		if (!mem)
			return EXIT_FAILURE;
		int bufsz = 256;
		string input;
		string tmp_str;
		string mem_str;
		while (TRUE)
		{
			printf("~#");
			input = readline(64, stdin);
			if (!input)
				continue;

			switch (*input)
			{
			case '!':
				mem_destr(mem);
				str_destr(input);
				return EXIT_SUCCESS;
				break;
			case '$':
				mem_str = mem_repr(mem);
				if (!mem_str)
					break;
				printf("%s\n", mem_str);
				str_destr(mem_str);
				break;
			case '@':
				mem_destr(mem);
				mem = mem_new();
				if (!mem)
				{
					str_destr(input);
					return -1;
				}
				break;
			}
			while (code_brackets_count(input) > 0)
			{
				printf("~~");
				tmp_str = readline(64, stdin);
				if (!tmp_str)
					continue;
				str_append(&input, tmp_str);
				str_destr(tmp_str);
			}
			evaluate_bf(input, &mem);
			str_destr(input);
		}
	}
	else
	{
		FILE* file = NULL;
		fopen_s(&file, argv[1], "r");
		if (!file)
		{
			printf("Error opening file.\n");
			return -1;
		}
		BFMem mem = mem_new();
		if (!mem)
			return -1;
		string input;
		string tmp_str;
		int op_count = 0;
		int evaluate_out;
		while (!feof(file))
		{
			input = readline(64, file);
			if (!input)
				break;
			while (code_brackets_count(input) > 0)
			{
				tmp_str = readline(64, file);
				if (!tmp_str)
					break;
				str_append(&input, tmp_str);
				str_destr(tmp_str);
			}
			evaluate_out = evaluate_bf(input, &mem);
			if (evaluate_out >= 0)
				op_count += evaluate_out;
			else
				return -1;
			str_destr(input);
		}
		printf("Executed %d operations.\n", op_count);
	}
	return EXIT_SUCCESS;
}