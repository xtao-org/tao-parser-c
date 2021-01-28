#include <string.h>
#include <stdlib.h>

typedef int Bool;

extern inline void freeptr(void** ptr) {
  void* value = *ptr;
  free(value);
  ptr = NULL;
}

typedef struct Node Node;
struct Node {
  Node* next;
  void* value;
};
Node* Node_make(void* value);
void Node_free(Node** nodeptr, void (*value_free)(void**));
void Node_bind(Node* node, Node* next);
Bool Node_isValid(Node* node);
void* Node_value(Node* node);
Node* Node_next(Node* node);

typedef struct {
  Node* head;
} Stack;
Stack* Stack_make();
void Stack_free(Stack** stackptr);
void Stack_push(Stack* stack, void* value);
void* Stack_pop(Stack* stack);
Bool Stack_empty(Stack* stack);

typedef struct {
  Node* head;
  Node* tail;
} List;
List* List_make();
void List_free(List** listptr, void (*value_free)(void**));
void List_add(List* list, void* value);
Node* List_head(List* list);

typedef struct {
  char* str;
  int capacity;
  int length;
} String;
String* String_make();
void String_free(String** stringptr);
String* String_make_c(char c);
void String_append(String* string, String* s);
void String_append_c(String* string, char c);
void String_append_cstr(String* string, char* s);
char* String_cstr(String* string);

inline Node* Node_make(void* value) {
  Node* node = (Node*)malloc(sizeof *node);
  node->value = value;
  node->next = NULL;
  return node;
}
inline void Node_free(Node** nodeptr, void (*value_free)(void**)) {
  Node* node = *nodeptr;
  value_free(&node->value);
  free(node);
  nodeptr = NULL;
}
inline void Node_bind(Node* node, Node* next) {
  node->next = next;
}
inline Bool Node_isValid(Node* node) {
  return node != NULL;
}
inline void* Node_value(Node* node) {
  return node->value;
}
inline Node* Node_next(Node* node) {
  return node->next;
}

inline List* List_make() {
  List* list = (List*)malloc(sizeof *list);
  list->head = NULL;
  list->tail = NULL;
  return list;
}
inline void List_free(List** listptr, void (*value_free)(void**)) {
  List* list = *listptr;
  Node* node = list->head;
  while (Node_isValid(node)) {
    Node* next = Node_next(node);
    Node_free(&node, value_free);
    node = next;
  }
  free(list);
  listptr = NULL;
}
inline void List_add(List* list, void* value) {
  Node* node = Node_make(value);
  if (list->tail == NULL || list->head == NULL) {
    list->head = node;
    list->tail = node;
  } else {
    Node_bind(list->tail, node);
    list->tail = node;
  }
}
inline Node* List_head(List* list) {
  return list->head;
}

inline Stack* Stack_make() {
  Stack* stack = (Stack*)malloc(sizeof *stack);
  stack->head = NULL;
  return stack;
}
inline void Stack_free(Stack** stackptr) {
  Stack* stack = *stackptr;
  Node* node = stack->head;
  if (Node_isValid(node)) {
    Node* next = Node_next(node);
    Node_free(&node, &freeptr);
    node = next;
  }
  free(stack);
  stackptr = NULL;
}
inline void Stack_push(Stack* stack, void* value) {
  Node* node = Node_make(value);
  if (stack->head == NULL) {
    stack->head = node;
  } else {
    Node_bind(node, stack->head);
    stack->head = node;
  }
}
inline void* Stack_pop(Stack* stack) {
  Node* node = stack->head;
  stack->head = node->next;
  void* value = node->value;
  free(node);
  return value;
}
extern inline void* Stack_peek(Stack* stack) {
  Node* node = stack->head;
  return node->value;
}
inline int Stack_size(Stack* stack) {
  Node* node = stack->head;
  int i = 0;
  for (; Node_isValid(node); ++i) {
    node = Node_next(node);
  }
  return i;
}
inline Bool Stack_empty(Stack* stack) {
  return stack->head == NULL;
}

inline String* String_make() {
  String* ret = (String*)malloc(sizeof *ret);
  int capacity = 64;
  ret->str = (char*)calloc(capacity, sizeof(char));
  ret->capacity = capacity;
  ret->length = 0;
  return ret;
}
inline void String_free(String** stringptr) {
  String* string = *stringptr;
  free(string->str);
  free(string);
  stringptr = NULL;
}
inline String* String_make_c(char c) {
  String* ret = String_make();
  String_append_c(ret, c);
  return ret;
}
inline void String_append_c(String* string, char c) {
  int length = string->length;
  if (length >= string->capacity - 1) {
    int capacity = string->capacity * 2;
    string->str = (char*)realloc(string->str, capacity);
    string->capacity = capacity;
  }
  string->str[length] = c;
  string->length = length + 1;
}
inline void String_append(String* string, String* s) {
  String_append_cstr(string, String_cstr(s));
}
inline void String_append_cstr(String* string, char* s) {
  for (int i = 0; i < strlen(s); ++i) {
    String_append_c(string, s[i]);
  }
}
extern inline char* String_to_cstr(String* string) {
  char* ret = (char*)calloc(string->length + 1, sizeof(char));
  strcpy(ret, string->str);
  return ret;
}
inline char* String_cstr(String* string) {
  return string->str;
}