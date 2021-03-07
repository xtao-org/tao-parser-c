#include <stdio.h>
#include <string.h>
#include "lib.h"

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
void Input_free(Input** inputptr);
Bool Input_done(Input* input);
Bool Input_at(Input* input, char symbol);
char Input_next(Input* input);
void Input_error(Input* input, const char* name);
void Input_bound(Input* input, char symbol);
void Input_unbound(Input* input);
Bool Input_atBound(Input* input);
Bool Input_meta(Input* input);

typedef Plist Part;
void Part_free(Part** partptr);
String* Part_unparse(Part* part);

typedef Plist Tao;
Tao* Tao_make();
List* Tao_tao(Tao* tao);
void Tao_free(Tao** taoptr);
List* Tao_parse_cstr(char* str);
List* Tao_parse(Input* input);
void Tao_push(Tao* tao, void* part);
String* Tao_unparse(Tao* tao);

typedef Plist Tree;
Tree* Tree_make(Tao* tao);
Tao* Tree_tree(Tree* tree);
void Tree_free(Tree** treeptr);
Bool Tree_isTree(Plist* tree);
Part* Tree_parse(Input* input);

typedef Plist Note;
Note* Note_make(char* note);
char* Note_note(Note* note);
void Note_free(Note** noteptr);
Bool Note_isNote(Plist* tree);
Part* Note_parse(Input* input);

typedef Plist Op;
Op* Op_make(char op);
char* Op_op(Op* op);
void Op_free(Op** opptr);
Bool Op_isOp(Plist* list);
Part* Op_parse(Input* input);

typedef char Other;
Other* Other_get();
Bool Other_isOther(void* value);

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
inline void Input_free(Input** inputptr) {
  Input* input = *inputptr;
  // free(input->str);
  Stack_free(&input->bounds);
  free(input);
  inputptr = NULL;
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

inline void Part_free(Part** partptr) {
  Part* part = *partptr;
  if (Note_isNote(part)) {
    Note_free((Note**)partptr);
    return;
  }
  if (Op_isOp(part)) {
    Op_free((Op**)partptr);
    return;
  }
  if (Tree_isTree(part)) {
    Tree_free((Tree**)partptr);
    return;
  }
  printf("Unrecognized part: %p", part);
  exit(1);
}
inline String* Part_unparse(Part* part) {
  if (Note_isNote(part)) {
    Note* note = (Note*)part;
    String* ret = String_make();
    String_append_cstr(ret, Note_note(note));
    return ret;
  }
  if (Op_isOp(part)) {
    Op* op = (Op*)part;
    String* ret = String_make_c('`');
    String_append_cstr(ret, Op_op(op));
    return ret;
  }
  if (Tree_isTree(part)) {
    Tree* tree = (Tree*)part;
    String* str = String_make_c('[');
    String* unparsed = Tao_unparse(Tree_tree(tree));
    String_append(str, unparsed);
    String_free(&unparsed);
    String_append_c(str, ']');
    return str;
  }
  printf("Unrecognized part: %p", part);
  exit(1);
}

inline Tao* Tao_make() {
  Tao* ret = Plist_make();
  Plist_set(ret, "tao", List_make());
  return ret;
}
inline List* Tao_tao(Tao* tao) {
  return (List*)Plist_get(tao, "tao");
}
inline void Tao_free(Tao** taoptr) {
  Tao* tao = *taoptr;
  List* parts = Tao_tao(tao);
  List_free(&parts, (void (*)(void**))&Part_free);
  free(tao);
  taoptr = NULL;
}
inline Tao* Tao_parse_cstr(char* str) {
  Input* input = Input_make(str);
  Tao* ret = Tao_parse(input);
  Input_free(&input);
  return ret;
}
inline Tao* Tao_parse(Input* input) {
  Tao* tao = Tao_make();
  while (1) {
    if (Input_atBound(input)) return tao;
    Part* part = Tree_parse(input);
    if (Other_isOther(part)) {
      part = Op_parse(input);
      if (Other_isOther(part)) {
        part = Note_parse(input);
      }
    }
    Tao_push(tao, part);
  }
  return tao;
}
inline void Tao_push(Tao* tao, void* part) {
  List_add(Tao_tao(tao), part);
}
inline String* Tao_unparse(Tao* tao) {
  String* str = String_make();
  List* parts = Tao_tao(tao);
  Node* node = List_head(parts);
  while (Node_isValid(node)) {
    String* unparsed = Part_unparse((List*)Node_value(node));
    String_append(str, unparsed);
    String_free(&unparsed);
    node = Node_next(node);
  }
  return str;
}

inline Tree* Tree_make(Tao* tao) {
  Tao* ret = Plist_make();
  Plist_set(ret, "tree", tao);
  return ret;
}
inline Tao* Tree_tree(Tree* tree) {
  return (Tao*)Plist_get(tree, "tree");
}
inline void Tree_free(Tree** treeptr) {
  Tree* tree = *treeptr;
  Tao* tao = Tree_tree(tree);
  Tao_free(&tao);
  free(tree);
  treeptr = NULL;
}
inline Bool Tree_isTree(Plist* tree) {
  return Tree_tree(tree) != NULL;
}
inline Part* Tree_parse(Input* input) {
  if (Input_at(input, '[')) {
    Input_next(input);
    Input_bound(input, ']');
    Tao* t = Tao_parse(input);
    Input_unbound(input);
    Input_next(input);
    return (Part*)Tree_make(t);
  }
  return (Part*)Other_get();
}

inline Op* Op_make(char ch) {
  char* op = (char*)malloc(sizeof(char) * 2);
  op[0] = ch;
  op[1] = 0;
  List* ret = Plist_make();
  Plist_set(ret, "op", op);
  return ret;
}
inline char* Op_op(Op* op) {
  return (char*)Plist_get(op, "op");
}
inline void Op_free(Op** opptr) {
  Op* op = *opptr;
  char* opstr = Op_op(op);
  free(opstr);
  free(op);
  opptr = NULL;
}
inline Bool Op_isOp(Plist* list) {
  return Op_op(list) != NULL;
}
inline Part* Op_parse(Input* input) {
  if (Input_at(input, '`')) {
    Input_next(input);
    if (Input_done(input)) Input_error(input, "op");
    return (Part*)Op_make(Input_next(input));
  }
  return (Part*)Other_get();
}

inline Note* Note_make(char* note) {
  Note* ret = Plist_make();
  Plist_set(ret, "note", note);
  return ret;
}
inline char* Note_note(Note* note) {
  return (char*)Plist_get(note, "note");
}
inline void Note_free(Note** noteptr) {
  Note* note = *noteptr;
  char* str = Note_note(note);
  free(str);
  free(note);
  // Plist_free...;
  noteptr = NULL;
}
inline Bool Note_isNote(Plist* note) {
  return Note_note(note) != NULL;
}
inline Part* Note_parse(Input* input) {
  if (Input_meta(input)) Input_error(input, "note");
  String* note = String_make_c(Input_next(input));
  while (1) {
    if (Input_meta(input) || Input_done(input)) {
      char* str = String_to_cstr(note);
      String_free(&note);
      return (Part*)Note_make(str);
    }
    String_append_c(note, Input_next(input));
  }
}

inline Other* Other_get() {
  static Other* ret = NULL;
  static const char* Other_str = "other";
  if (ret == NULL) {
    Other* ret = (Other*)malloc(sizeof(char) * strlen(Other_str) + 1);
    ret = strcpy(ret, Other_str);
  }
  return ret;
}
inline Bool Other_isOther(void* value) {
  return Other_get() == value;
}