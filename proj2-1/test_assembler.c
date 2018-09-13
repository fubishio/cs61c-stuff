#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include <CUnit/Basic.h>

#include "src/utils.h"
#include "src/tables.h"
#include "src/translate_utils.h"
#include "src/translate.h"

const char* TMP_FILE = "test_output.txt";
const int BUF_SIZE = 1024;
const char* TRANSLATE_TEST= "test_translate.txt";
const char* TRANSLATE_TEST1= "test_translate1.txt";
const char* TRANSLATE_TEST2 = "test_translate2.txt";

/****************************************
 *  Helper functions 
 ****************************************/

int do_nothing() {
    return 0;
}

int init_log_file() {
    set_log_file(TMP_FILE);
    return 0;
}

int check_lines_equal(char **arr, int num) {
    char buf[BUF_SIZE];

    FILE *f = fopen(TMP_FILE, "r");
    if (!f) {
        CU_FAIL("Could not open temporary file");
        return 0;
    }
    for (int i = 0; i < num; i++) {
        if (!fgets(buf, BUF_SIZE, f)) {
            CU_FAIL("Reached end of file");
            return 0;
        }
        CU_ASSERT(!strncmp(buf, arr[i], strlen(arr[i])));
    }
    fclose(f);
    return 0;
}

int check_lines_equal_translate(char **arr, int num) {
    char buf[BUF_SIZE];

    FILE *f = fopen(TRANSLATE_TEST, "r");
    if (!f) {
        CU_FAIL("Could not open temporary file");
        return 0;
    }
    for (int i = 0; i < num; i++) {
        if (!fgets(buf, BUF_SIZE, f)) {
            printf("1");
            CU_FAIL("Reached end of file");
            return 0;
        }
        // printf("%s\n", buf);
        // printf("%s\n", arr[i]);
        CU_ASSERT(!strncmp(buf, arr[i], strlen(arr[i])));
    }
    fclose(f);
    return 0;
}

int check_lines_equal_translate1(char **arr, int num) {
    char buf[BUF_SIZE];

    FILE *f = fopen(TRANSLATE_TEST1, "r");
    if (!f) {
        CU_FAIL("Could not open temporary file");
        return 0;
    }
    for (int i = 0; i < num; i++) {
        if (!fgets(buf, BUF_SIZE, f)) {
            printf("1");
            CU_FAIL("Reached end of file");
            return 0;
        }
        // printf("%s\n", buf);
        // printf("%s\n", arr[i]);
        CU_ASSERT(!strncmp(buf, arr[i], strlen(arr[i])));
    }
    fclose(f);
    return 0;
}


int check_lines_equal_translate2(char **arr, int num) {
    char buf[BUF_SIZE];

    FILE *f = fopen(TRANSLATE_TEST2, "r");
    if (!f) {
        CU_FAIL("Could not open temporary file");
        return 0;
    }
    for (int i = 0; i < num; i++) {
        if (!fgets(buf, BUF_SIZE, f)) {
            CU_FAIL("Reached end of file");
            return 0;
        }
        // printf("%s\n", buf);
        // printf("%s\n", arr[i]);
        CU_ASSERT(!strncmp(buf, arr[i], strlen(arr[i])));
    }
    fclose(f);
    return 0;
}
/****************************************
 *  Test cases for translate_utils.c 
 ****************************************/

void test_translate_reg() {
    CU_ASSERT_EQUAL(translate_reg("$0"), 0);
    CU_ASSERT_EQUAL(translate_reg("$at"), 1);
    CU_ASSERT_EQUAL(translate_reg("$v0"), 2);
    CU_ASSERT_EQUAL(translate_reg("$a0"), 4);
    CU_ASSERT_EQUAL(translate_reg("$a1"), 5);
    CU_ASSERT_EQUAL(translate_reg("$a2"), 6);
    CU_ASSERT_EQUAL(translate_reg("$a3"), 7);
    CU_ASSERT_EQUAL(translate_reg("$t0"), 8);
    CU_ASSERT_EQUAL(translate_reg("$t1"), 9);
    CU_ASSERT_EQUAL(translate_reg("$t2"), 10);
    CU_ASSERT_EQUAL(translate_reg("$t3"), 11);
    CU_ASSERT_EQUAL(translate_reg("$s0"), 16);
    CU_ASSERT_EQUAL(translate_reg("$s1"), 17);
    CU_ASSERT_EQUAL(translate_reg("$3"), -1);
    CU_ASSERT_EQUAL(translate_reg("asdf"), -1);
    CU_ASSERT_EQUAL(translate_reg("hey there"), -1);
}

void test_translate_num() {
    long int output;

    CU_ASSERT_EQUAL(translate_num(&output, "35", -1000, 1000), 0);
    CU_ASSERT_EQUAL(output, 35);
    CU_ASSERT_EQUAL(translate_num(&output, "145634236", 0, 9000000000), 0);
    CU_ASSERT_EQUAL(output, 145634236);
    CU_ASSERT_EQUAL(translate_num(&output, "0xC0FFEE", -9000000000, 9000000000), 0);
    CU_ASSERT_EQUAL(output, 12648430);
    CU_ASSERT_EQUAL(translate_num(&output, "72", -16, 72), 0);
    CU_ASSERT_EQUAL(output, 72);
    CU_ASSERT_EQUAL(translate_num(&output, "72", -16, 71), -1);
    CU_ASSERT_EQUAL(translate_num(&output, "72", 72, 150), 0);
    CU_ASSERT_EQUAL(output, 72);
    CU_ASSERT_EQUAL(translate_num(&output, "72", 73, 150), -1);
    CU_ASSERT_EQUAL(translate_num(&output, "35x", -100, 100), -1);
}

/****************************************
 *  Test cases for tables.c 
 ****************************************/

void test_table_1() {
    int retval;

    SymbolTable* tbl = create_table(SYMTBL_UNIQUE_NAME);
    CU_ASSERT_PTR_NOT_NULL(tbl);

    retval = add_to_table(tbl, "abc", 8);
    CU_ASSERT_EQUAL(retval, 0);
    retval = add_to_table(tbl, "efg", 12);
    CU_ASSERT_EQUAL(retval, 0);
    retval = add_to_table(tbl, "q45", 16);
    CU_ASSERT_EQUAL(retval, 0);
    retval = add_to_table(tbl, "q45", 24); 
    CU_ASSERT_EQUAL(retval, -1); 
    retval = add_to_table(tbl, "bob", 14); 
    CU_ASSERT_EQUAL(retval, -1); 

    retval = get_addr_for_symbol(tbl, "abc");
    CU_ASSERT_EQUAL(retval, 8); 
    retval = get_addr_for_symbol(tbl, "q45");
    CU_ASSERT_EQUAL(retval, 16); 
    retval = get_addr_for_symbol(tbl, "ef");
    CU_ASSERT_EQUAL(retval, -1);
    
    free_table(tbl);

    char* arr[] = { "Error: name 'q45' already exists in table.",
                    "Error: address is not a multiple of 4." };
    check_lines_equal(arr, 2);

    SymbolTable* tbl2 = create_table(SYMTBL_NON_UNIQUE);
    CU_ASSERT_PTR_NOT_NULL(tbl2);

    retval = add_to_table(tbl2, "q45", 16);
    CU_ASSERT_EQUAL(retval, 0);
    retval = add_to_table(tbl2, "q45", 24); 
    CU_ASSERT_EQUAL(retval, 0);

    free_table(tbl2);

}

void test_table_2() {
    int retval, max = 100;

    SymbolTable* tbl = create_table(SYMTBL_UNIQUE_NAME);
    CU_ASSERT_PTR_NOT_NULL(tbl);

    char buf[10];
    for (int i = 0; i < max; i++) {
        sprintf(buf, "%d", i);
        retval = add_to_table(tbl, buf, 4 * i);
        CU_ASSERT_EQUAL(retval, 0);
    }

    for (int i = 0; i < max; i++) {
        sprintf(buf, "%d", i);
        retval = get_addr_for_symbol(tbl, buf);
        CU_ASSERT_EQUAL(retval, 4 * i);
    }

    free_table(tbl);
}

void test_translate() {
    FILE *translate_test = fopen(TRANSLATE_TEST, "w");
    int retval;
    char *temp[3] = {"$v0", "$t1", "$t2"};   
    retval = write_rtype(0x21, translate_test, temp, 3);
    CU_ASSERT_EQUAL(retval, 0);
    retval = write_rtype(0x25, translate_test, temp, 3);
    CU_ASSERT_EQUAL(retval, 0);
    char *temp1[3] = {"$t0", "$s0", "$s1"};
    retval = write_rtype(0x2a, translate_test, temp1, 3);
    CU_ASSERT_EQUAL(retval, 0);
    char *temp2[3] = {"$t0", "$s0", "$s1"};
    retval = write_rtype(0x2b, translate_test, temp2, 3);
    CU_ASSERT_EQUAL(retval, 0);

    char *temp3[1] = {"$at"};
    retval = write_jr(0x08, translate_test, temp3, 1);
    CU_ASSERT_EQUAL(retval, 0);
    char *temp4[3] = {"$a3", "$t1", "5"};
    retval = write_shift(0x00, translate_test, temp4, 3);
    CU_ASSERT_EQUAL(retval, 0);
    char *temp5[3] = {"$t3", "$sp", "9"};
    retval = write_addiu(0x09, translate_test, temp5, 3);
    CU_ASSERT_EQUAL(retval, 0);
    char *temp6[3] = {"$a1", "$s1", "11"};
    retval = write_ori(0x0d, translate_test, temp6, 3);
    CU_ASSERT_EQUAL(retval, 0);
    char *temp7[2] = {"$a2", "110"};
    retval = write_lui(0x0f, translate_test, temp7, 2);
    CU_ASSERT_EQUAL(retval, 0);
    char *temp8[3] = {"$t3", "1", "$v0"};
    retval = write_mem(0x20, translate_test, temp8, 3);
    CU_ASSERT_EQUAL(retval, 0);
    char *temp9[3] = {"$a2", "6", "$t1"};
    retval = write_mem(0x24, translate_test, temp9, 3);
    CU_ASSERT_EQUAL(retval, 0);
    char *temp10[3] = {"$0", "3", "$s1"};
    retval = write_mem(0x23, translate_test, temp10, 3);
    CU_ASSERT_EQUAL(retval, 0);


    char *temp11[3] = {"$s1", "81", "$0"};
    retval = write_mem(0x28, translate_test, temp11, 3);
    CU_ASSERT_EQUAL(retval, 0);
    char *temp12[3] = {"$s2", "3", "$t1"};
    retval = write_mem(0x2b, translate_test, temp12, 3);
    CU_ASSERT_EQUAL(retval, 0);


    fclose(translate_test);
    char* temptest[] = {"012a1021\n", "012a1025\n", "0211402a\n", "0211402b\n", "00200008\n",
    "00093940\n", "27ab0009\n", "3625000b\n", "3c06006e\n", "804b0001\n", "91260006\n", "8e200003\n",
    "a0110051\n", "ad320003\n"};
    

    check_lines_equal_translate1(temptest, 14);
}


void test_translate1() {
    FILE *translate_test1 = fopen(TRANSLATE_TEST1, "w");
    SymbolTable* symtbl = create_table(SYMTBL_UNIQUE_NAME);
    SymbolTable* reltbl = create_table(SYMTBL_NON_UNIQUE);
    add_to_table(symtbl, "try", 0);
    add_to_table(symtbl, "try1", 16);
    add_to_table(symtbl, "error", 40);

    int retval;
    char *temp[3] = {"$v0", "$t1", "$t2"};
    translate_inst(translate_test1, "addu", temp, 3, 0, symtbl, reltbl);   
    translate_inst(translate_test1, "or", temp, 3, 0, symtbl, reltbl);
    char *temp1[3] = {"$t0", "$s0", "$s1"};
    translate_inst(translate_test1, "slt", temp1, 3, 0, symtbl, reltbl);
    char *temp2[3] = {"$t0", "$s0", "$s1"};
    translate_inst(translate_test1, "sltu", temp2, 3, 0, symtbl, reltbl);
    char *temp3[1] = {"$at"};
    translate_inst(translate_test1, "jr", temp3, 1, 0, symtbl, reltbl);
    char *temp4[3] = {"$a3", "$t1", "5"};
    translate_inst(translate_test1, "sll", temp4, 3, 0, symtbl, reltbl);
    char *temp5[3] = {"$t3", "$sp", "9"};
    translate_inst(translate_test1, "addiu", temp5, 3, 0, symtbl, reltbl);
    char *temp6[3] = {"$a1", "$s1", "11"};
    translate_inst(translate_test1, "ori", temp6, 3, 0, symtbl, reltbl);
    char *temp7[2] = {"$a2", "110"};
    translate_inst(translate_test1, "lui", temp7, 2, 0, symtbl, reltbl);
    char *temp8[3] = {"$t3", "1", "$v0"};
    translate_inst(translate_test1, "lb", temp8, 3, 0, symtbl, reltbl);
    char *temp9[3] = {"$a2", "6", "$t1"};
    translate_inst(translate_test1, "lbu", temp9, 3, 0, symtbl, reltbl);
    char *temp10[3] = {"$0", "3", "$s1"};
    translate_inst(translate_test1, "lw", temp10, 3, 0, symtbl, reltbl);
    char *temp11[3] = {"$s1", "81", "$0"};
    translate_inst(translate_test1, "sb", temp11, 3, 0, symtbl, reltbl);
    char *temp12[3] = {"$s2", "3", "$t1"};
    translate_inst(translate_test1, "sw", temp12, 3, 0, symtbl, reltbl);




    char *temp13[3] = {"$s1", "$s3", "try"};
    translate_inst(translate_test1, "beq", temp13, 3, 4, symtbl, reltbl);
    char *temp14[3] = {"$s1", "$s3", "try"};
    translate_inst(translate_test1, "beq", temp14, 3, 8, symtbl, reltbl);
    char *temp15[3] = {"$s1", "$s3", "try1"};
    translate_inst(translate_test1, "beq", temp15, 3, 12, symtbl, reltbl);

    char *temp16[3] = {"$s1", "$s2", "error"};
    translate_inst(translate_test1, "bne", temp16, 3, 20, symtbl, reltbl);
    char *temp17[3] = {"$s1", "$s2", "error"};
    translate_inst(translate_test1, "bne", temp17, 3, 24, symtbl, reltbl);

    char *temp18[1] = {"error"};
    translate_inst(translate_test1, "j", temp18, 1, 28, symtbl, reltbl);
    char *temp19[1] = {"try1"};
    translate_inst(translate_test1, "j", temp19, 1, 32, symtbl, reltbl);
    char *temp20[1] = {"error"};
    translate_inst(translate_test1, "jal", temp20, 1, 36, symtbl, reltbl);

    free_table(symtbl);
    free_table(reltbl);

    fclose(translate_test1);
    char* temptest1[] = {"012a1021\n",
    "012a1025\n",    "0211402a\n",
    "0211402b\n",    "00200008\n",
    "00093940\n",    "27ab0009\n",
    "3625000b\n",    "3c06006e\n",
    "804b0001\n",    "91260006\n",
    "8e200003\n",    "a0110051\n",
    "ad320003\n",    "1233ffff\n",
    "1233fffe\n",    "12330000\n",
    "16320004\n",    "16320003\n",
    "08000000\n",    "08000000\n",
    "0c000000\n"};
    check_lines_equal_translate1(temptest1, 22);
}

void test_translate2() {
    FILE *translate_test2 = fopen(TRANSLATE_TEST2, "w");
    char* tran100[2] = {"$v0", "5"};
    write_pass_one(translate_test2, "li", tran100, 2);
    char* tran101[2] = {"$v0", "348536"};
    write_pass_one(translate_test2, "li", tran101, 2);
    char* tran102[2] = {"$v0", "$v2"};
    write_pass_one(translate_test2, "move", tran102, 2);
    char* tran103[3] = {"$v0","$v1", "$v2"};
    write_pass_one(translate_test2, "traddu", tran103, 3);
    char* tran104[2] = {"$v0","$v1"};
    write_pass_one(translate_test2, "swpr", tran104, 2);
    char* tran105[3] = {"$v0", "$v1", "banana"};
    write_pass_one(translate_test2, "blt", tran105, 3);
    char* tran106[3] = {"$v0", "$v1", "tamarind"};
    write_pass_one(translate_test2, "bgt", tran106, 3);

        // char* tran7[3] = {"$at,", comma, "$at"};
        // write_inst_string(output, "or", tran7, 3);
        // char* tran8[3] = {comma, comma, "32"};
        // write_inst_string(output, "sll", tran8, 3);
        // char* tran9[3] = {comma, comma, args[1]};
        // write_inst_string(output, "or", tran9, 3);
        // char* tran10[3] = {comma1, comma1, "32"};
        // write_inst_string(output, "sll", tran10, 3);
        // char* tran11[3] = {comma1, comma1, "$at"};
        // write_inst_string(output, "or", tran11, 3);
        // char* tran12[3] = {"$at,", "$at,", "32"};
        // write_inst_string(output, "sll", tran12, 3);


    // fprintf(translate_test2, "%d\n", (348536 >> 16));
    // fprintf(translate_test2, "%d\n", (348536 & 0x0000ffff));

    
    fclose(translate_test2);
    char* temptest2[] = {"addiu $v0, 5\n", "lui $v0, 5\n", "ori $v0, $v0, 20856\n",
                        "addiu $v0, $v2, $0\n", "addu $v0, $v0, $v1\n", "addu $v0, $v0, $v2\n",
                        "or $at, $v0, $at\n", "sll $v0, $v0, 32\n", "or $v0, $v0, $v1\n",
                        "sll $v1, $v1, 32\n", "or $v1, $v1, $at\n", "sll $at, $at, 32\n",
                        "slt $at, $v0, $v1\n", "addiu $s1, $0, 1\n", "beq $at, $s1, banana\n",
                        "sll $s1, $s1, 32\n", "sll $at, $at, 32\n",

                    "slt $at, $v0, $v1\n",
                    "beq $at, $s1, done\n",
                    "blt $v0, $v1, done\n",
                    "bne $0, $0, tamarind\n",
                    "sll $at, $at, 32\n",
                    "done:\n",
                    "sll $at, $at, 32\n"};





    check_lines_equal_translate2(temptest2, 24);
}


/****************************************
 *  Add your test cases here
 ****************************************/

int main(int argc, char** argv) {
    CU_pSuite pSuite1 = NULL, pSuite2 = NULL, pSuite3 = NULL;

    if (CUE_SUCCESS != CU_initialize_registry()) {
        return CU_get_error();
    }

    /* Suite 1 */
    pSuite1 = CU_add_suite("Testing translate_utils.c", NULL, NULL);
    if (!pSuite1) {
        goto exit;
    }
    if (!CU_add_test(pSuite1, "test_translate_reg", test_translate_reg)) {
        goto exit;
    }
    if (!CU_add_test(pSuite1, "test_translate_num", test_translate_num)) {
        goto exit;
    }

    /* Suite 2 */
    pSuite2 = CU_add_suite("Testing tables.c", init_log_file, NULL);
    if (!pSuite2) {
        goto exit;
    }
    if (!CU_add_test(pSuite2, "test_table_1", test_table_1)) {
        goto exit;
    }
    if (!CU_add_test(pSuite2, "test_table_2", test_table_2)) {
        goto exit;
    }

    pSuite3 = CU_add_suite("Testing translate.c", NULL, NULL);
    if (!pSuite3) {
        goto exit;
    }
    if (!CU_add_test(pSuite3, "test_translate", test_translate)) {
        goto exit;
    }
    if (!CU_add_test(pSuite3, "test_translate1", test_translate1)) {
        goto exit;
    }
    if (!CU_add_test(pSuite3, "test_translate2", test_translate2)) {
        goto exit;
    }


    CU_basic_set_mode(CU_BRM_VERBOSE);
    CU_basic_run_tests();

exit:
    CU_cleanup_registry();
    return CU_get_error();;
}
