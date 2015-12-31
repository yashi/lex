#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>

enum _itemType {
	itemError,
	itemLeftMeta,
	itemRightMeta,
	itemInside,
	itemText,
	itemDot,
	itemEOF,
};
typedef enum _itemType itemType;

struct Lexer {
	unsigned int width;
	unsigned int start;
	unsigned int pos;
	unsigned int cap;
	char *input;
};

char leftMeta[] = "{{";
char rightMeta[] = "}}";

void* lexText(struct Lexer *l);
void* lexLeftMeta(struct Lexer *l);
void* lexrightMeta(struct Lexer *l);
void* lexInsideAction(struct Lexer *l);

void lexer_emit(struct Lexer *l, itemType type)
{
	char buf[128];
	unsigned int len = l->pos - l->start;

	memcpy(buf, l->input + l->start, len);
	buf[len] = '\0';
	printf("%d: `%s'\n", type, buf);
	l->start = l->pos;
}

char lexer_next(struct Lexer *l)
{
	if (l->pos >= l->cap) {
		l->width = 0;
		return '\0';
	}
	l->pos++;
	return l->input[l->pos];
}

void lexer_errorf(struct Lexer *l, char *format, ...)
{
	va_list args;

	(void)l;
	va_start(args, format);
	vfprintf(stderr, format, args);
	va_end(args);
}

void *lexLeftMeta(struct Lexer *l)
{
	l->pos += 2;
	lexer_emit(l, itemLeftMeta);
	return lexInsideAction;
}

void* lexRightMeta(struct Lexer *l)
{
	l->pos += 2;
	lexer_emit(l, itemRightMeta);
	return lexText;
}

void* lexInsideAction(struct Lexer *l)
{
	for (;;) {
		if (strncmp(l->input + l->pos, rightMeta, 2) == 0) {
			lexer_emit(l, itemInside);
			return lexRightMeta;
		}
		if (lexer_next(l) == '\0') break;
	}
	lexer_errorf(l, "unclosed action\n");
	return NULL;
}

void* lexText(struct Lexer *l) {
	for (;;) {
		if (strncmp(l->input + l->pos, leftMeta, 2) == 0) {
			if (l->pos > l->start) {
				lexer_emit(l, itemText);
			}
			return lexLeftMeta;
		}
		if (lexer_next(l) == '\0') break;
	}
	// Correctly reached EOF
	if (l->pos > l->start) {
		lexer_emit(l, itemText);
	}
	return NULL;
}

void lexer_run(struct Lexer *l) {
	void *(*state)(struct Lexer *l);
	for (state = lexText; state != NULL; ) {
		state = state(l);
	}
}

#define BUFSIZE 10
int main(void)
{
	struct Lexer *l;
	l = calloc(sizeof(*l), 1);
	l->input = "hello {{ world }}.";
	l->cap = strlen(l->input);

	lexer_run(l);
	return 0;
}
