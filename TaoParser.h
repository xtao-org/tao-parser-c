#include <stdio.h>
#include "lib.h"

// todo: check if more getters needed & implement if so
// todo: clean up, organize; perhaps extract decls to header file; +separate List, Node, Stack, String

typedef struct {
  int position;
  char symbol;
} Bound;
Bound* Bound_make(int position, char symbol);
int Bound_position(Bound* bound);
char Bound_symbol(Bound* bound);

typedef struct {
  int position;
  char* str;
  int length;
  Stack* bounds;
} Input;
Input* Input_make(char* str);
void Input_free(Input* input);
Bool Input_done(Input* input);
Bool Input_at(Input* input, char symbol);
char Input_next(Input* input);
void Input_error(Input* input, const char* name);
void Input_bound(Input* input, char symbol);
void Input_unbound(Input* input);
Bool Input_atBound(Input* input);
Bool Input_meta(Input* input);

typedef struct {
  const char* tag;
} Tagged;
const char* Tagged_tag(Tagged* tagged);
void Tagged_free(Tagged* tagged);
String* Tagged_unparse(Tagged* value);

typedef struct {
  const char* tag;
  List* parts;
} Tao;
Tao* Tao_make();
void Tao_free(Tao* tao);
Tao* Tao_parse_cstr(char* str);
Tao* Tao_parse(Input* input);
void Tao_push(Tao* tao, void* part);
String* Tao_unparse(Tao* tao);

typedef struct {
  const char* tag;
  Tao* tao;
} Tree;
Tree* Tree_make(Tao* tao);
void Tree_free(Tree* tree);
Tagged* Tree_parse(Input* input);

typedef struct {
  const char* tag;
  char* note;
} Note;
Note* Note_make(char* note);
void Note_free(Note* note);
Tagged* Note_parse(Input* input);

typedef struct {
  const char* tag;
  char op;
} Op;
Op* Op_make(char op);
void Op_free(Op* op);
Tagged* Op_parse(Input* input);

typedef struct {
  const char* tag;
} Other;
Other* Other_make();

inline Bound* Bound_make(int position, char symbol) {
  Bound* bound = (Bound*)malloc(sizeof *bound);
  bound->position = position;
  bound->symbol = symbol;
  return bound;
}
inline int Bound_position(Bound* bound) {
  return bound->position;
}
inline char Bound_symbol(Bound* bound) {
  return bound->symbol;
}

inline Input* Input_make(char* str) {
  Input* ret = (Input*)malloc(sizeof *ret);
  ret->position = 0;
  ret->str = str;
  ret->length = strlen(str);
  ret->bounds = Stack_make();
  return ret;
}
inline void Input_free(Input* input) {
  // free(input->str);
  Stack_free(input->bounds);
  free(input);
  input = NULL;
}
inline Bool Input_done(Input* input) {
  return input->position >= input->length;
}
inline Bool Input_at(Input* input, char symbol) {
  return input->str[input->position] == symbol;
}
inline char Input_next(Input* input) {
  return input->str[input->position++];
}
inline void Input_error(Input* input, const char* name) {
  printf("Error: malformed %s at %d", name, input->position);
  exit(1);
}
inline void Input_bound(Input* input, char symbol) {
  Bound* bound = Bound_make(input->position, symbol);
  Stack_push(input->bounds, bound);
}
inline void Input_unbound(Input* input) {
  free(Stack_pop(input->bounds));
}
inline Bool Input_atBound(Input* input) {
  Stack* bounds = input->bounds;
  if (!Stack_empty(bounds)) {
    Bound* bound = (Bound*)Stack_peek(bounds);
    int position = Bound_position(bound);
    char symbol = Bound_symbol(bound);
    if (Input_done(input)) {
      printf("Error: since %d expected \"%c\" before end of input\n", position, symbol);
      exit(1);
    }
    return Input_at(input, symbol);
  }
  return Input_done(input);
}
inline Bool Input_meta(Input* input) {
  return Input_at(input, '[') || Input_at(input, '`') || Input_at(input, ']');
}

inline const char* Tagged_tag(Tagged* tagged) {
  return tagged->tag;
}
inline void Tagged_free(Tagged* tagged) {
  const char* tag = Tagged_tag(tagged);
  if (strcmp(tag, "note") == 0) {
    Note_free((Note*)tagged);
    return;
  }
  if (strcmp(tag, "op") == 0) {
    Op_free((Op*)tagged);
    return;
  }
  if (strcmp(tag, "tree") == 0) {
    Tree_free((Tree*)tagged);
    return;
  }
  printf("Unrecognized tag: %s", tag);
  exit(1);
}
inline String* Tagged_unparse(Tagged* value) {
  const char* tag = Tagged_tag(value);
  if (strcmp(tag, "note") == 0) {
    Note* note = (Note*)value;
    String* ret = String_make();
    String_append_cstr(ret, note->note);
    return ret;
  }
  if (strcmp(tag, "op") == 0) {
    Op* op = (Op*)value;
    String* ret = String_make_c('`');
    String_append_c(ret, op->op);
    return ret;
  }
  if (strcmp(tag, "tree") == 0) {
    Tree* tree = (Tree*)value;
    String* str = String_make_c('[');
    String* unparsed = Tao_unparse(tree->tao);
    String_append(str, unparsed);
    String_free(unparsed);
    String_append_c(str, ']');
    return str;
  }
  printf("Unrecognized tag: %s", tag);
  exit(1);
}

inline Tao* Tao_make() {
  Tao* ret = (Tao*)malloc(sizeof *ret);
  ret->tag = "tao";
  ret->parts = List_make();
  return ret;
}
inline void Tao_free(Tao* tao) {
  List_free(tao->parts, (void (*)(void*))&Tagged_free);
  free(tao);
  tao = NULL;
}
inline Tao* Tao_parse_cstr(char* str) {
  Input* input = Input_make(str);
  Tao* ret = Tao_parse(input);
  Input_free(input);
  return ret;
}
inline Tao* Tao_parse(Input* input) {
  Tao* tao = Tao_make();
  while (1) {
    if (Input_atBound(input)) return tao;
    Tagged* part = Tree_parse(input);
    if (strcmp(Tagged_tag(part), "other") == 0) {
      part = Op_parse(input);
      if (strcmp(Tagged_tag(part), "other") == 0) {
        part = Note_parse(input);
      }
    }
    Tao_push(tao, part);
  }
  return tao;
}
inline void Tao_push(Tao* tao, void* part) {
  List_add(tao->parts, part);
}
inline String* Tao_unparse(Tao* tao) {
  String* str = String_make();
  List* parts = tao->parts;
  Node* node = List_head(parts);
  while (node != NULL) {
    String* unparsed = Tagged_unparse((Tagged*)Node_value(node));
    String_append(str, unparsed);
    String_free(unparsed);
    node = Node_next(node);
  }
  return str;
}

inline Tree* Tree_make(Tao* tao) {
  Tree* ret = (Tree*)malloc(sizeof *ret);
  ret->tag = "tree";
  ret->tao = tao;
  return ret;
}
inline void Tree_free(Tree* tree) {
  Tao_free(tree->tao);
  free(tree);
  tree = NULL;
}
inline Tagged* Tree_parse(Input* input) {
  if (Input_at(input, '[')) {
    Input_next(input);
    Input_bound(input, ']');
    Tao* t = Tao_parse(input);
    Input_unbound(input);
    Input_next(input);
    return (Tagged*)Tree_make(t);
  }
  return (Tagged*)Other_make();
}

inline Op* Op_make(char op) {
  Op* ret = (Op*)malloc(sizeof *ret);
  ret->tag = "op";
  ret->op = op;
  return ret;
}
inline void Op_free(Op* op) {
  free(op);
  op = NULL;
}
inline Tagged* Op_parse(Input* input) {
  if (Input_at(input, '`')) {
    Input_next(input);
    if (Input_done(input)) Input_error(input, "op");
    return (Tagged*)Op_make(Input_next(input));
  }
  return (Tagged*)Other_make();
}

inline Note* Note_make(char* note) {
  Note* ret = (Note*)malloc(sizeof *ret);
  ret->tag = "note";
  ret->note = note;
  return ret;
}
inline void Note_free(Note* note) {
  free(note->note);
  free(note);
  note = NULL;
}
inline Tagged* Note_parse(Input* input) {
  if (Input_meta(input)) Input_error(input, "note");
  String* note = String_make_c(Input_next(input));
  while (1) {
    if (Input_meta(input) || Input_done(input)) {
      char* str = String_to_cstr(note);
      String_free(note);
      return (Tagged*)Note_make(str);
    }
    String_append_c(note, Input_next(input));
  }
}

static inline Other* Other_make_once() {
  Other* ret = (Other*)malloc(sizeof *ret);
  ret->tag = "other";
  return ret;
}
inline Other* Other_make() {
  static Other* ret = NULL;
  if (ret == NULL) ret = Other_make_once();
  return ret;
}