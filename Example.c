#include <stdio.h>
#include <assert.h>
#include "TaoParser.h"

int main(void)
{
  char* input = "key [value] key2 [value2] k3`[`]`` [[v3][v4][v5]]";
  Tao* tao = Tao_parse_cstr(input);
  String* unparsed = Tao_unparse(tao);
  char* output = String_cstr(unparsed);
  assert(strcmp(input, output) == 0);
  printf("%s\n", output);
  String_free(&unparsed);
  Tao_free(&tao);
  return 0;
}