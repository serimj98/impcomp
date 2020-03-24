#include <assert.h>
#include <stdio.h>
#include <limits.h>
#include <stdlib.h>

#include "lib/xalloc.h"
#include "lib/stack.h"
#include "lib/contracts.h"
#include "lib/c0v_stack.h"
#include "lib/c0vm.h"
#include "lib/c0vm_c0ffi.h"
#include "lib/c0vm_abort.h"

/* call stack frames */
typedef struct frame_info frame;
struct frame_info {
  c0v_stack_t S; /* Operand stack of C0 values */
  ubyte *P;      /* Function body */
  size_t pc;     /* Program counter */
  c0_value *V;   /* The local variables */
};

int execute(struct bc0_file *bc0) {
  REQUIRES(bc0 != NULL);

  /* Variables */
  c0v_stack_t S = c0v_stack_new(); /* Operand stack of C0 values */
  ubyte *P = bc0->function_pool->code; /* Array of bytes that make up the current function */
  size_t pc = 0; /* Current location within the current byte array P */
  c0_value *V = xcalloc(bc0->function_pool->num_vars, sizeof(c0_value)); /* Local variables (you won't need this till Task 2) */
  //(void) V;

  /* The call stack, a generic stack that should contain pointers to frames */
  /* You won't need this until you implement functions. */
  gstack_t callStack = stack_new();
  //(void) callStack;

  while (true) {

#ifdef DEBUG
    /* You can add extra debugging information here */
    fprintf(stderr, "Opcode %x -- Stack size: %zu -- PC: %zu\n",
            P[pc], c0v_stack_size(S), pc);
#endif

    switch (P[pc]) {

    /* Additional stack operation: */

    case POP: { //+1
      pc++;
      c0v_pop(S);
      break;
    }

    case DUP: { //+1
      pc++;
      c0_value v = c0v_pop(S);
      c0v_push(S,v);
      c0v_push(S,v);
      break;
    }

    case SWAP: { //+1
      pc++;
      c0_value v2 = c0v_pop(S);
      c0_value v1 = c0v_pop(S);
      c0v_push(S,v2);
      c0v_push(S,v1);
      break;
    }


    /* Returning from a function.
     * This currently has a memory leak! You will need to make a slight
     * change for the initial tasks to avoid leaking memory.  You will
     * need to be revise it further when you write INVOKESTATIC. */

    case RETURN: { //+1
      pc++;
      //if we're using invokestatic's callStack
      if (!stack_empty(callStack)) {
        c0_value iniret = c0v_pop(S);
        free(V);
        c0v_stack_free(S);
        frame* pres = (frame*)(pop(callStack));
        P = pres->P;
        pc = pres->pc;
        V = pres->V;
        S = pres->S;
        free(pres);
        c0v_push(S, iniret);
        break;
      }

      int retval = val2int(c0v_pop(S));
      assert(c0v_stack_empty(S));

#ifdef DEBUG
      fprintf(stderr, "Returning %d from execute()\n", retval);

#endif
      // Free everything before returning from the execute function!
      stack_free(callStack, NULL);
      free(V);
      c0v_stack_free(S);
      return retval;
    }


    /* Arithmetic and Logical operations */

    case IADD: { //+1
      pc++;
      c0_value c0y = c0v_pop(S);
      c0_value c0x = c0v_pop(S);
      int32_t y = val2int(c0y);
      int32_t x = val2int(c0x);
      int32_t v = x + y;
      c0_value c0v = int2val(v);
      c0v_push(S,c0v);
      break;
    }

    case ISUB: { //+1
      pc++;
      c0_value c0y = c0v_pop(S);
      c0_value c0x = c0v_pop(S);
      int32_t y = val2int(c0y);
      int32_t x = val2int(c0x);
      int32_t v = x - y;
      c0_value c0v = int2val(v);
      c0v_push(S,c0v);
      break;
    }

    case IDIV: { //+1
      pc++;
      c0_value c0y = c0v_pop(S);
      c0_value c0x = c0v_pop(S);
      int32_t y = val2int(c0y);
      int32_t x = val2int(c0x);
      if (y == 0) c0_arith_error("IDIV: Division by 0.");
      if (x == -2147483648 && y == -1) c0_arith_error("IDIV: Division by -1.");
      int32_t v = x / y;
      c0_value c0v = int2val(v);
      c0v_push(S,c0v);
      break;
    }

    case IREM: { //+1
      pc++;
      c0_value c0y = c0v_pop(S);
      c0_value c0x = c0v_pop(S);
      int32_t y = val2int(c0y);
      int32_t x = val2int(c0x);
      if (y == 0) c0_arith_error("IREM: Division by 0.");
      if (x == -2147483648 && y == -1) c0_arith_error("IREM: Division by -1.");
      int32_t v = x % y;
      c0_value c0v = int2val(v);
      c0v_push(S,c0v);
      break;
    }

    case IMUL: { //+1
      pc++;
      c0_value c0y = c0v_pop(S);
      c0_value c0x = c0v_pop(S);
      int32_t y = val2int(c0y);
      int32_t x = val2int(c0x);
      int32_t v = x * y;
      c0_value c0v = int2val(v);
      c0v_push(S,c0v);
      break;
    }

    case IAND: { //+1
      pc++;
      c0_value c0y = c0v_pop(S);
      c0_value c0x = c0v_pop(S);
      int32_t y = val2int(c0y);
      int32_t x = val2int(c0x);
      int32_t v = x & y;
      c0_value c0v = int2val(v);
      c0v_push(S,c0v);
      break;
    }

    case IOR: { //+1
      pc++;
      c0_value c0y = c0v_pop(S);
      c0_value c0x = c0v_pop(S);
      int32_t y = val2int(c0y);
      int32_t x = val2int(c0x);
      int32_t v = x | y;
      c0_value c0v = int2val(v);
      c0v_push(S,c0v);
      break;
    }

    case IXOR: { //+1
      pc++;
      c0_value c0y = c0v_pop(S);
      c0_value c0x = c0v_pop(S);
      int32_t y = val2int(c0y);
      int32_t x = val2int(c0x);
      int32_t v = x ^ y;
      c0_value c0v = int2val(v);
      c0v_push(S,c0v);
      break;
    }

    case ISHL: { //+1
      pc++;
      c0_value c0y = c0v_pop(S);
      c0_value c0x = c0v_pop(S);
      int32_t y = val2int(c0y);
      int32_t x = val2int(c0x);
      if (y < 0) c0_arith_error("ISHL: Illegal shift (negative).");
      if (y > 31) c0_arith_error("ISHL: Illegal shift (> 31).");
      int32_t v = x << y;
      c0_value c0v = int2val(v);
      c0v_push(S,c0v);
      break;
    }

    case ISHR: { //+1
      pc++;
      c0_value c0y = c0v_pop(S);
      c0_value c0x = c0v_pop(S);
      int32_t y = val2int(c0y);
      int32_t x = val2int(c0x);
      if (y < 0) c0_arith_error("ISHR: Illegal shift (negative).");
      if (y > 31) c0_arith_error("ISHR: Illegal shift (> 31).");
      int32_t v = x >> y;
      c0_value c0v = int2val(v);
      c0v_push(S,c0v);
      break;
    }


    /* Pushing constants */

    case BIPUSH: { //+2
      pc++;
      int8_t b = P[pc];
      pc++;
      int32_t x = (int32_t)b; //casting from small to big; sign extends
      c0_value c0v = int2val(x);
      c0v_push(S,c0v);
      break;
    }

    case ILDC: { //+3
      pc++;
      uint32_t c1 = P[pc];
      pc++;
      uint32_t c2 = P[pc];
      pc++;
      int32_t x = (bc0->int_pool)[(c1<<8)|c2];
      c0_value c0v = int2val(x);
      c0v_push(S,c0v);
      break;
    }

    case ALDC: { //+3
      pc++;
      uint32_t c1 = P[pc];
      pc++;
      uint32_t c2 = P[pc];
      pc++;
      char* x = &(bc0->string_pool)[(c1<<8)|c2];
      c0_value c0v = ptr2val(x);
      c0v_push(S,c0v);
      break;
    }

    case ACONST_NULL: { //+1
      pc++;
      void* x = NULL;
      c0_value c0v = ptr2val(x);
      c0v_push(S,c0v);
      break;
    }


    /* Operations on local variables */

    case VLOAD: { //+2
      pc++;
      uint8_t i = P[pc];
      pc++;
      c0_value v = V[i];
      c0v_push(S,v);
      break;
    }

    case VSTORE: { //+2
      pc++;
      uint8_t i = P[pc];
      pc++;
      c0_value v = c0v_pop(S);
      V[i] = v;
      break;
    }


    /* Control flow operations */

    case NOP: { //+1
      pc++;
      break;
    }

    case IF_CMPEQ: { //+3
      c0_value v2 = c0v_pop(S);
      c0_value v1 = c0v_pop(S);
      int32_t o1 = (int32_t)(int8_t)P[pc+1];
      int32_t o2 = (int32_t)(uint32_t)P[pc+2];
      if (val_equal(v1,v2)) {
        pc = pc + (o1<<8|o2);
      }
      else {
        pc++;
        pc++;
        pc++;
      }
      break;
    }
 
    case IF_CMPNE: { //+3
      c0_value v2 = c0v_pop(S);
      c0_value v1 = c0v_pop(S);
      int32_t o1 = (int32_t)(int8_t)P[pc+1];
      int32_t o2 = (int32_t)(uint32_t)P[pc+2];
      if (!val_equal(v1,v2)) {
        pc = pc + (o1<<8|o2);
      }
      else {
        pc++;
        pc++;
        pc++;
      }
      break;
    }

    case IF_ICMPLT: { //+3
      c0_value c0y = c0v_pop(S);
      c0_value c0x = c0v_pop(S);
      int32_t y = val2int(c0y);
      int32_t x = val2int(c0x);
      int32_t o1 = (int32_t)(int8_t)P[pc+1];
      int32_t o2 = (int32_t)(uint32_t)P[pc+2];
      if (x < y) {
        pc = pc + (o1<<8|o2);
      }
      else {
        pc++;
        pc++;
        pc++;
      }
      break;
    }

    case IF_ICMPGE: { //+3
      c0_value c0y = c0v_pop(S);
      c0_value c0x = c0v_pop(S);
      int32_t y = val2int(c0y);
      int32_t x = val2int(c0x);
      int32_t o1 = (int32_t)(int8_t)P[pc+1];
      int32_t o2 = (int32_t)(uint32_t)P[pc+2];
      if (x >= y) {
        pc = pc + (o1<<8|o2);
      }
      else {
        pc++;
        pc++;
        pc++;
      }
      break;
    }

    case IF_ICMPGT: { //+3
      c0_value c0y = c0v_pop(S);
      c0_value c0x = c0v_pop(S);
      int32_t y = val2int(c0y);
      int32_t x = val2int(c0x);
      int32_t o1 = (int32_t)(int8_t)P[pc+1];
      int32_t o2 = (int32_t)(uint32_t)P[pc+2];
      if (x > y) {
        pc = pc + (o1<<8|o2);
      }
      else {
        pc++;
        pc++;
        pc++;
      }
      break;
    }

    case IF_ICMPLE: { //+3
      c0_value c0y = c0v_pop(S);
      c0_value c0x = c0v_pop(S);
      int32_t y = val2int(c0y);
      int32_t x = val2int(c0x);
      int32_t o1 = (int32_t)(int8_t)P[pc+1];
      int32_t o2 = (int32_t)(uint32_t)P[pc+2];
      if (x <= y) {
        pc = pc + (o1<<8|o2);
      }
      else {
        pc++;
        pc++;
        pc++;
      }
      break;
    }

      //(int32_t)(int8_t)P[pc]; //if you want sign extend (preserve value)
      //(int32_t)(uint32_t)P[pc]; //if you don't want sign extend (preserve bits)

    case GOTO: { //+3
      int32_t o1 = (int32_t)(int8_t)P[pc+1];
      int32_t o2 = (int32_t)(uint32_t)P[pc+2];
      pc = pc + ((o1<<8)|o2);
      break;
    }

    case ATHROW: { //+1
      pc++;
      c0_value c0a = c0v_pop(S);
      char* a = (char*)val2ptr(c0a); //is this how to get the string?
      c0_user_error(a);
      break;
    }

    case ASSERT: { //+1
      pc++;
      c0_value c0a = c0v_pop(S);
      c0_value c0x = c0v_pop(S);
      int32_t x = val2int(c0x);
      if (x == 0) {
        char* a = (char*)val2ptr(c0a);
        c0_assertion_failure(a);
      }
      break;
    }


    /* Function call operations: */

    case INVOKESTATIC: { //+3
      //access function g from function pool
      pc++;
      uint32_t c1 = P[pc];
      pc++;
      uint32_t c2 = P[pc];
      pc++;
      struct function_info fi = bc0->function_pool[c1<<8|c2];

      //save state of calling function in order to resume later
      frame* F  = xmalloc(sizeof(frame));
      F->S = S;
      F->P = P;
      F->pc = pc;
      F->V = V;
      push(callStack, (void*)F); //typedef void *stack_elem

      //new locals array populated with arguments
      c0_value *V_new = xcalloc(fi.num_vars, sizeof(c0_value));
      for (int i = 0; i < fi.num_args; i++) {
        V_new[fi.num_args-i-1] = c0v_pop(S);
      }
      V = V_new;
      S = c0v_stack_new();
      P = fi.code;
      pc = 0;

      break;
    }

    case INVOKENATIVE: { //+3
      //access function g from native pool
      pc++;
      uint32_t c1 = P[pc];
      pc++;
      uint32_t c2 = P[pc];
      pc++;
      struct native_info ni = bc0->native_pool[c1<<8|c2];
      native_fn *nf = native_function_table[ni.function_table_index];
      
      //construct array of c0 values to input to native_fn
      c0_value *garr = xcalloc(ni.num_args, sizeof(c0_value));
      for (int i = 0; i < ni.num_args; i++) {
        garr[ni.num_args-i-1] = c0v_pop(S);
      }

      c0v_push(S, nf(garr));
      break;
    }


    /* Memory allocation operations: */

    case NEW: { //+2
      pc++;
      uint32_t s = P[pc];
      pc++;
      void* mem = xcalloc(1, s);
      c0_value c0mem = ptr2val(mem);
      c0v_push(S, c0mem);
      break;
    }

    case NEWARRAY: { //+2
      pc++;
      uint32_t s = P[pc];
      pc++;
      c0_value c0n = c0v_pop(S);
      uint32_t n = val2int(c0n);
      c0_array* a = xcalloc(1, sizeof(c0_array));
      a->count = n;
      a->elt_size = s;
      void* elems_arr = xcalloc(n, s);
      a->elems = elems_arr;
      c0_value c0a = ptr2val(a);
      c0v_push(S, c0a);
      break;
    }

    case ARRAYLENGTH: { //+1
      pc++;
      c0_value c0a = c0v_pop(S);
      c0_array* a = val2ptr(c0a);
      uint32_t n = a->count;
      c0_value c0n = int2val(n);
      c0v_push(S, c0n);
      break;
    }


    /* Memory access operations: */

    case AADDF: { //+2
      pc++;
      uint8_t f = P[pc];
      pc++;
      c0_value c0a = c0v_pop(S);
      char* a = val2ptr(c0a);
      if (a == NULL) c0_memory_error("AADDF: a is NULL.");
      a = a + f;
      c0_value c0af = ptr2val(a);
      c0v_push(S, c0af);
      break;
    }

    case AADDS: { //+1
      pc++;
      c0_value c0i = c0v_pop(S);
      c0_value c0a = c0v_pop(S);
      uint32_t i = val2int(c0i);
      c0_array* a = val2ptr(c0a);
      if (a == NULL) c0_memory_error("AADDS: a is NULL.");
      char* anew = (char*)(a->elems)+(a->elt_size)*i;
      c0_value c0aesi = ptr2val(anew);
      c0v_push(S, c0aesi);
      break;
    }

    case IMLOAD: { //+1
      pc++;
      c0_value c0a = c0v_pop(S);
      void* a = val2ptr(c0a);
      if (a == NULL) c0_memory_error("IMLOAD: a is NULL.");
      int32_t* x = (int*)a;
      c0_value c0x = int2val(*x);
      c0v_push(S, c0x);
      break;
    }

    case IMSTORE: { //+1
      pc++;
      c0_value c0x = c0v_pop(S);
      c0_value c0a = c0v_pop(S);
      int32_t x = val2int(c0x);
      int32_t* a = val2ptr(c0a);
      if (a == NULL) c0_memory_error("IMSTORE: a is NULL.");
      *a = x;
      break;
    }

    case AMLOAD: { //+1
      pc++;
      c0_value c0a = c0v_pop(S);
      void** a = val2ptr(c0a);
      if (a == NULL) c0_memory_error("AMLOAD: a is NULL.");
      void* b = *a;
      c0_value c0b = ptr2val(b);
      c0v_push(S, c0b);
      break;
    }

    case AMSTORE: { //+1
      pc++;
      c0_value c0b = c0v_pop(S);
      c0_value c0a = c0v_pop(S);
      void* b = val2ptr(c0b);
      void** a = val2ptr(c0a);
      if (a == NULL) c0_memory_error("AMSTORE: a is NULL.");
      *a = b;
      break;
    }

    case CMLOAD: { //+1
      pc++;
      c0_value c0a = c0v_pop(S);
      void* a = val2ptr(c0a);
      if (a == NULL) c0_memory_error("CMLOAD: a is NULL.");
      int32_t* x = (int*)(a);
      c0_value c0x = int2val(*x);
      c0v_push(S, c0x);
      break;
    }

    case CMSTORE: { //+1
      pc++;
      c0_value c0x = c0v_pop(S);
      c0_value c0a = c0v_pop(S);
      int32_t* a = val2ptr(c0a);
      if (a == NULL) c0_memory_error("CMSTORE: a is NULL.");
      int32_t x = val2int(c0x) & 0x7f;
      *a = x;
      break;
    }

    default:
      fprintf(stderr, "invalid opcode: 0x%02x\n", P[pc]);
      abort();
    }
  }

  /* cannot get here from infinite loop */
  assert(false);
}
