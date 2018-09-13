#include <stdio.h>
#include <string.h>

#include <unistd.h>
#include <sys/stat.h>

#include "beargit.h"
#include "util.h"

/* Implementation Notes:
 *
 * - Functions return 0 if successful, 1 if there is an error.
 * - All error conditions in the function description need to be implemented
 *   and written to stderr. We catch some additional errors for you in main.c.
 * - Output to stdout needs to be exactly as specified in the function description.
 * - Only edit this file (beargit.c)
 * - Here are some of the helper functions from util.h:
 *   * fs_mkdir(dirname): create directory <dirname>
 *   * fs_rm(filename): delete file <filename>
 *   * fs_mv(src,dst): move file <src> to <dst>, overwriting <dst> if it exists
 *   * fs_cp(src,dst): copy file <src> to <dst>, overwriting <dst> if it exists
 *   * write_string_to_file(filename,str): write <str> to filename (overwriting contents)
 *   * read_string_from_file(filename,str,size): read a string of at most <size> (incl.
 *     NULL character) from file <filename> and store it into <str>. Note that <str>
 *     needs to be large enough to hold that string.
 *  - You NEED to test your code. The autograder we provide does not contain the
 *    full set of tests that we will run on your code. See "Step 5" in the project spec.
 */

/* beargit init
 *
 * - Create .beargit directory
 * - Create empty .beargit/.index file
 * - Create .beargit/.prev file containing 0..0 commit id
 *
 * Output (to stdout):
 * - None if successful
 */

int counter = 0;
int index_counter = 0;

int beargit_init(void) {
  fs_mkdir(".beargit");

  FILE* findex = fopen(".beargit/.index", "w");
  fclose(findex);

  FILE* fbranches = fopen(".beargit/.branches", "w");
  fprintf(fbranches, "%s\n", "master");
  fclose(fbranches);

  write_string_to_file(".beargit/.prev", "0000000000000000000000000000000000000000");
  write_string_to_file(".beargit/.current_branch", "master");

  return 0;
}



/* beargit add <filename>
 *
 * - Append filename to list in .beargit/.index if it isn't in there yet
 *
 * Possible errors (to stderr):
 * >> ERROR:  File <filename> has already been added.
 *
 * Output (to stdout):
 * - None if successful
 */

int beargit_add(const char* filename) {
  FILE* findex = fopen(".beargit/.index", "r");
  FILE* fnewindex = fopen(".beargit/.newindex", "w");

  char line[FILENAME_SIZE];
  while(fgets(line, sizeof(line), findex)) {
    strtok(line, "\n");
    if (strcmp(line, filename) == 0) {
      fprintf(stderr, "ERROR:  File %s has already been added.\n", filename);
      fclose(findex);
      fclose(fnewindex);
      fs_rm(".beargit/.newindex");
      return 1;
    }
    fprintf(fnewindex, "%s\n", line);
  }
  fprintf(fnewindex, "%s\n", filename);
  fclose(findex);
  fclose(fnewindex);

  fs_mv(".beargit/.newindex", ".beargit/.index");

  return 0;
}

/* beargit status
 *
 * See "Step 1" in the project spec.
 *
 */

int beargit_status() {
  FILE* findex = fopen(".beargit/.index", "r");

  char line[FILENAME_SIZE];
  fprintf(stdout, "Tracked files:\n\n");

  int i = 0;
  while(fgets(line, sizeof(line), findex)) {
    printf("%s", line);
    i++;
  }

  fprintf(stdout, "\nThere are %d files total.\n", i);
  fclose(findex);
  return 0;
}

/* beargit rm <filename>
 *
 * See "Step 2" in the project spec.
 *
 */

int beargit_rm(const char* filename) {
  FILE *findex = fopen(".beargit/.index", "r");
  FILE *fnewindex = fopen(".beargit/.newindex", "w");
  char line[FILENAME_SIZE];
  int tf = 1;
  while(fgets(line, sizeof(line), findex)) {
    strtok(line, "\n");
    if (strcmp(line, filename) == 0) {
      tf = 0;
      continue;
    }
    fprintf(fnewindex, "%s\n", line);
  }
  if (tf) {
    fprintf(stderr, "ERROR:  File %s not tracked.\n", filename);
    fclose(findex);
    fclose(fnewindex);
    fs_rm(".beargit/.newindex");
    return 1;    
  } else {
    fclose(findex);
    fclose(fnewindex);

    fs_mv(".beargit/.newindex", ".beargit/.index");

    return 0;
  }
}

/* beargit commit -m <msg>
 *
 * See "Step 3" in the project spec.
 *
 */

const char* go_bears = "THIS IS BEAR TERRITORY!";

int is_commit_msg_ok(const char* msg) {
  const char* mp = msg;
  while (*mp) {
    const char* bp = go_bears;
    while (*mp && *bp && *mp == *bp) {
      bp++;
      mp++;
    }
    if (!*bp) {
        return 1;
    }
    mp++;
  }
  return 0;
}

/* Use next_commit_id to fill in the rest of the commit ID.
 *
 * Hints:
 * You will need a destination string buffer to hold your next_commit_id, before you copy it back to commit_id
 * You will need to use a function we have provided for you.
 */

void next_commit_id(char* commit_id) {
    char temphash[COMMIT_ID_SIZE];
    cryptohash(commit_id, temphash);
    strcpy(commit_id, temphash);
}

int beargit_commit(const char* msg) {
  char err1[50+BRANCHNAME_SIZE];
  read_string_from_file(".beargit/.current_branch", err1, BRANCHNAME_SIZE);
  if (strlen(err1) == 0) {
    fprintf(stderr, "ERROR: Need to be on HEAD of a branch to commit\n");
    return 1;
  }
  if (!is_commit_msg_ok(msg)) {
    fprintf(stderr, "ERROR:  Message must contain \"%s\"\n", go_bears);
    return 1;
  }

  char commit_id[50 + COMMIT_ID_SIZE];
  read_string_from_file(".beargit/.prev", commit_id, COMMIT_ID_SIZE);
  next_commit_id(commit_id);

  char new_dir[50 + COMMIT_ID_SIZE];
  sprintf(new_dir, ".beargit/%s", commit_id);
  // strcpy(new_dir, ".beargit/");
  // strncat(new_dir, commit_id, COMMIT_ID_SIZE);
  fs_mkdir(new_dir);

  char new_dir_index[50 + COMMIT_ID_SIZE];
  sprintf(new_dir_index, ".beargit/%s/.index", commit_id);
  // strcpy(new_dir_index,".beargit/");
  // strncat(new_dir_index, commit_id, COMMIT_ID_SIZE);
  // strncat(new_dir_index, "/.index", 7);
  fs_cp(".beargit/.index", new_dir_index);
  
  char new_dir_prev[50 + COMMIT_ID_SIZE];
  sprintf(new_dir_prev, ".beargit/%s/.prev", commit_id);
  // strcpy(new_dir_prev,".beargit/");
  // strncat(new_dir_prev, commit_id, COMMIT_ID_SIZE);
  // strncat(new_dir_prev, "/.prev", 6);
  fs_cp(".beargit/.prev", new_dir_prev);

  //new_dir is now technically the path to the .msg file
  strncat(new_dir, "/.msg", 5);
  write_string_to_file(new_dir, msg);
  write_string_to_file(".beargit/.prev", commit_id);

  char temp_dir_path[50 + COMMIT_ID_SIZE];
  char temp_file_path[50 + COMMIT_ID_SIZE + FILENAME_SIZE];
  char line[FILENAME_SIZE + 50];
  // char sourced[FILENAME_SIZE+50];

  sprintf(temp_dir_path, ".beargit/%s/", commit_id);
  // strncat(temp_dir_path, ".beargit/", 9);
  // strncat(temp_dir_path, commit_id, COMMIT_ID_SIZE);
  // strncat(temp_dir_path, "/", 1);
  strcpy(temp_file_path, temp_dir_path);
  
  FILE *findex = fopen(".beargit/.index", "r");

  while(fgets(line, sizeof(line), findex)) {
    strtok(line, "\n");
    strcat(temp_file_path, line);
    fs_cp(line, temp_file_path);
    strcpy(temp_file_path, temp_dir_path);
  }
  fclose(findex);

  return 0;
}
// int beargit_commit(const char* msg) {
//   char err1[BRANCHNAME_SIZE];
//   read_string_from_file(".beargit/.current_branch", err1, BRANCHNAME_SIZE);
//   if (strlen(err1) == 0) {
//     fprintf(stderr, "ERROR: Need to be on HEAD of a branch to commit\n");
//     return 1;
//   }
//   if (!is_commit_msg_ok(msg)) {
//     fprintf(stderr, "ERROR:  Message must contain \"%s\"\n", go_bears);
//     return 1;
//   }

//   char commit_id[COMMIT_ID_SIZE];
//   read_string_from_file(".beargit/.prev", commit_id, COMMIT_ID_SIZE);
//   next_commit_id(commit_id);

//   char new_dir[50 + COMMIT_ID_SIZE];
//   strcpy(new_dir, ".beargit/");
//   strncat(new_dir, commit_id, COMMIT_ID_SIZE);
//   fs_mkdir(new_dir);

//   char new_dir_index[50 + COMMIT_ID_SIZE];
//   strcpy(new_dir_index,".beargit/");
//   strncat(new_dir_index, commit_id, COMMIT_ID_SIZE);
//   strncat(new_dir_index, "/.index", 7);
//   fs_cp(".beargit/.index", new_dir_index);
  
//   char new_dir_prev[50 + COMMIT_ID_SIZE];
//   strcpy(new_dir_prev,".beargit/");
//   strncat(new_dir_prev, commit_id, COMMIT_ID_SIZE);
//   strncat(new_dir_prev, "/.prev", 6);
//   fs_cp(".beargit/.prev", new_dir_prev);

//   //new_dir is now technically the path to the .msg file
//   strncat(new_dir, "/.msg", 5);
//   write_string_to_file(new_dir, msg);
//   write_string_to_file(".beargit/.prev", commit_id);

//   char temp_dir_path[10+COMMIT_ID_SIZE];
//   char temp_file_path[10+COMMIT_ID_SIZE+FILENAME_SIZE];
//   char line[FILENAME_SIZE];
//   char sourced[FILENAME_SIZE+2];

//   strncat(temp_dir_path, ".beargit/", 9);
//   strncat(temp_dir_path, commit_id, COMMIT_ID_SIZE);
//   strncat(temp_dir_path, "/", 1);
//   strcpy(temp_file_path, temp_dir_path);
  
//   FILE *findex = fopen(".beargit/.index", "r");

//   while(fgets(line, sizeof(line), findex)) {
//     strtok(line, "\n");

//     strncat(temp_file_path, line, FILENAME_SIZE);
//     strncat(sourced, "./", 2);
//     strncat(sourced, line, FILENAME_SIZE);
//     fs_cp(sourced, temp_file_path);
//     strcpy(temp_file_path, temp_dir_path);
//     strcpy(sourced, "");
//   }
//   fclose(findex);
//   return 0;
// }

/* beargit log
 *
 * See "Step 4" in the project spec.
 *
 */

int beargit_log(int limit) {
  int i = 0;
  char cur[COMMIT_ID_SIZE];
  read_string_from_file(".beargit/.prev", cur, COMMIT_ID_SIZE);
  if (strcmp(cur, "0000000000000000000000000000000000000000") == 0) {
    fprintf(stderr, "ERROR:  There are no commits.\n");
    return 1;
  }

  char tempID[COMMIT_ID_SIZE];
  char temp_prev_dir[9+COMMIT_ID_SIZE+6];
  char temp_msg_dir[9+COMMIT_ID_SIZE+5];
  char temp_msg[512];

  read_string_from_file(".beargit/.prev", tempID, COMMIT_ID_SIZE);
  fprintf(stdout, "commit %s\n", tempID);

  strcpy(temp_msg_dir, ".beargit/");
  strncat(temp_msg_dir, tempID, COMMIT_ID_SIZE);
  strncat(temp_msg_dir, "/.msg", 5);
  read_string_from_file(temp_msg_dir, temp_msg, 512);

  fprintf(stdout, "    %s\n\n", temp_msg);
  i++;

  while (i < limit) {
    strcpy(temp_prev_dir, ".beargit/");
    strncat(temp_prev_dir, tempID, COMMIT_ID_SIZE);
    strncat(temp_prev_dir, "/.prev", 6);

    read_string_from_file(temp_prev_dir, tempID, COMMIT_ID_SIZE);
    
    if(strcmp(tempID, "0000000000000000000000000000000000000000") == 0) {
      return 0;
    }
    
    strcpy(temp_msg_dir, ".beargit/");
    strncat(temp_msg_dir, tempID, COMMIT_ID_SIZE);
    strncat(temp_msg_dir, "/.msg", 5);
    read_string_from_file(temp_msg_dir, temp_msg, 512);

    fprintf(stdout, "commit %s\n", tempID);
    fprintf(stdout, "    %s\n\n", temp_msg);    
    i++;
  }

  fprintf(stdout, "\n");
  return 0;
}


// This helper function returns the branch number for a specific branch, or
// returns -1 if the branch does not exist.
int get_branch_number(const char* branch_name) {
  FILE* fbranches = fopen(".beargit/.branches", "r");

  int branch_index = -1;
  int i = 0;
  char line[FILENAME_SIZE];
  while(fgets(line, sizeof(line), fbranches)) {
    strtok(line, "\n");
    if (strcmp(line, branch_name) == 0) {
      branch_index = i;
    }
    i++;
  }

  fclose(fbranches);
  return branch_index;
}

/* beargit branch
 *
 * See "Step 5" in the project spec.
 *
 */

int beargit_branch() {
  FILE* fbranch = fopen(".beargit/.branches", "r");

  char line[50+BRANCHNAME_SIZE];
  char cur_branch[50+BRANCHNAME_SIZE];

  read_string_from_file(".beargit/.current_branch", cur_branch, BRANCHNAME_SIZE);

  while(fgets(line, sizeof(line), fbranch)) {
    strtok(line, "\n");
    if (strcmp(cur_branch, line) == 0) {
      fprintf(stdout, "*  %s\n", line);
    } else {
      fprintf(stdout, "   %s\n", line);
    }
  }

  fclose(fbranch);
  return 0;
}

/* beargit checkout
 *
 * See "Step 6" in the project spec.
 *
 */

int checkout_commit(const char* commit_id) {

  char line[FILENAME_SIZE];
  char temp_del[FILENAME_SIZE+50];
  FILE* findex = fopen(".beargit/.index", "r");
  while(fgets(line, sizeof(line), findex)) {
    strtok(line, "\n");
    /*strcpy(temp_del, "./");*/
    strcat(temp_del, line);
    fs_rm(line);
    /*strcpy(temp_del, "");*/
  }
  fclose(findex);

  if (commit_id == "0000000000000000000000000000000000000000") {
    write_string_to_file(".beargit/.index", "");
    write_string_to_file(".beargit/.prev", commit_id);
    return 0;
  }

  char index_dir[50+COMMIT_ID_SIZE];
  /*strncpy(index_dir, ".beargit/", 9);
  strncat(index_dir, commit_id, COMMIT_ID_SIZE);
  strncat(index_dir, "/.index", 7);*/
  sprintf(index_dir, ".beargit/%s/.index", commit_id);
  fs_cp(index_dir, ".beargit/.index");

  char temp_file_path[50+COMMIT_ID_SIZE+FILENAME_SIZE];
  sprintf(temp_file_path, ".beargit/%s", commit_id);
  // strncpy(temp_file_path, ".beargit/", 9);
  // strncat(temp_file_path, commit_id, COMMIT_ID_SIZE);
  
  FILE* temp_cp = fopen(".beargit/.index", "r");

  while(fgets(line, sizeof(line), temp_cp)) {
    strtok(line, "\n");
    sprintf(temp_file_path, ".beargit/%s/%s", commit_id, line);
    // strncat(temp_file_path, "/", 1);
    // strncat(temp_file_path, line, FILENAME_SIZE);
    
    fs_cp(temp_file_path, line);
    // strcpy(temp_file_path, ".beargit/");
    // strcat(temp_file_path, commit_id);
  }
  write_string_to_file(".beargit/.prev", commit_id);
  fclose(temp_cp);
  return 0;
}

int is_it_a_commit_id(const char* commit_id) {
  if (strlen(commit_id) != 40) {
    return 0;
  }
  char temp_dir_path1[50+COMMIT_ID_SIZE];
  strcat(temp_dir_path1, ".beargit/");
  strcat(temp_dir_path1, commit_id);
  return fs_check_dir_exists(temp_dir_path1);
}

int beargit_checkout(const char* arg, int new_branch) {
  /*FILE *fp;
  fp = freopen("myfile.txt", "w", stderr);*/

  // Get the current branch
  char current_branch[BRANCHNAME_SIZE+50];
  read_string_from_file(".beargit/.current_branch", current_branch, BRANCHNAME_SIZE);

  // If not detached, leave the current branch by storing the current HEAD into that branch's file...
  if (strlen(current_branch)) {
    char current_branch_file[BRANCHNAME_SIZE+50];
    strcpy(current_branch_file, ".beargit/.branch_");
    char* strcat(current_branch_file, current_branch);
    // sprintf(current_branch_file, ".beargit/.branch_%s", current_branch);
    fs_cp(".beargit/.prev", current_branch_file);
  }

   // Check whether the argument is a commit ID. If yes, we just change to detached mode
  // without actually having to change into any other branch.
  if (is_it_a_commit_id(arg)) {
    char commit_dir[FILENAME_SIZE+50] = ".beargit/";
    strcat(commit_dir, arg);
    // ...and setting the current branch to none (i.e., detached).
    write_string_to_file(".beargit/.current_branch", "");

    return checkout_commit(arg);
  }



  // Read branches file (giving us the HEAD commit id for that branch).
  int branch_exists = (get_branch_number(arg) >= 0);

  // Check for errors.
  if (!(!branch_exists || !new_branch)) {
    fprintf(stderr, "ERROR:  A branch named %s already exists.\n", arg);
    return 1;
  } else if (!branch_exists && !new_branch) {
    fprintf(stderr, "ERROR:  No branch or commit %s exists.\n", arg);
    return 1;
  }

  /*if (arg == "00.0") {
    return checkout_commit("00.0");
    return 0;
  }*/
  // Just a better name, since we now know the argument is a branch name.
  const char* branch_name = arg;

  // File for the branch we are changing into.
  char branch_file[BRANCHNAME_SIZE+50]/* = ".beargit/.branch_"*/;
  sprintf(branch_file, ".beargit/.branch_%s", branch_name);
  /*strncat(branch_file, branch_name, BRANCHNAME_SIZE);*/

  // Update the branch file if new branch is created (now it can't go wrong anymore)
  if (new_branch) {
    FILE* fbranches = fopen(".beargit/.branches", "a");
    fprintf(fbranches, "%s\n", branch_name);
    fclose(fbranches);
    fs_cp(".beargit/.prev", branch_file);
  }

  write_string_to_file(".beargit/.current_branch", branch_name);

  // Read the head commit ID of this branch.
  char branch_head_commit_id[COMMIT_ID_SIZE];
  read_string_from_file(branch_file, branch_head_commit_id, COMMIT_ID_SIZE);
  /*fclose(fp);*/
  // Check out the actual commit.
  return checkout_commit(branch_head_commit_id);
}

/* beargit reset
 *
 * See "Step 7" in the project spec.
 *
 */

int beargit_reset(const char* commit_id, const char* filename) {
  if (!is_it_a_commit_id(commit_id)) {
      fprintf(stderr, "ERROR:  Commit %s does not exist.\n", commit_id);
      return 1;
  } 

  // Check if the file is in the commit directory
  char temp_dir_path[50+COMMIT_ID_SIZE];
  strcpy(temp_dir_path, ".beargit/");
  strncat(temp_dir_path, commit_id, COMMIT_ID_SIZE);
  strncat(temp_dir_path, "/", 1);
  strncat(temp_dir_path, filename, FILENAME_SIZE);
  if(fs_check_dir_exists(temp_dir_path)) {
      fprintf(stderr, "ERROR:  %s is not in the index of commit %s.\n", filename, commit_id);
      return 1;
  }

  // Copy the file to the current working directory
  if (fs_check_dir_exists(filename)) {
    fs_rm(filename);
  }
  fs_cp(temp_dir_path, filename);


  // Add the file if it wasn't already there
  /* COMPLETE THIS PART */

  return 0;
}

/* beargit merge
 *
 * See "Step 8" in the project spec.
 *
 */

int beargit_merge(const char* arg) {
  // Get the commit_id or throw an error
  char commit_id[COMMIT_ID_SIZE];
  if (!is_it_a_commit_id(arg)) {
      if (get_branch_number(arg) == -1) {
            fprintf(stderr, "ERROR:  No branch or commit %s exists.\n", arg);
            return 1;
      }
      char branch_file[FILENAME_SIZE];
      snprintf(branch_file, FILENAME_SIZE, ".beargit/.branch_%s", arg);
      read_string_from_file(branch_file, commit_id, COMMIT_ID_SIZE);
  } else {
      snprintf(commit_id, COMMIT_ID_SIZE, "%s", arg);
  }

  // Iterate through each line of the commit_id index and determine how you
  // should copy the index file over
  char temp_merge_index[400];
  strcat(temp_merge_index, ".beargit/");
  strcat(temp_merge_index, commit_id);
  strcat(temp_merge_index, "/.index");

  char temp_merge_cpath[400];
  strcat(temp_merge_cpath, ".beargit/");
  strcat(temp_merge_cpath, commit_id);
  strcat(temp_merge_cpath, "/");

  FILE* cindex;
  char line3[FILENAME_SIZE];

  FILE* mindex = fopen(temp_merge_index, "r");
  char line5[FILENAME_SIZE];

  int yn = 0;
  char old_name[400];
  char new_name[400];

  FILE* src;
  FILE* new;

  char buffer[4096];
  int size;



  while(fgets(line5, sizeof(line5), mindex)) {
    strtok(line5, "\n");
    cindex = fopen(".beargit/.index", "a+");
    while(fgets(line3, sizeof(line3), cindex)) {
      strtok(line3, "\n");
      if (strcmp(line5, line3) == 0) {
        strcat(temp_merge_cpath, line5);
        strcat(new_name, line5);
        strcat(new_name, ".");
        strcat(new_name, commit_id);
        src = fopen(temp_merge_cpath, "r");
        new = fopen(new_name, "w");
        while ((size = fread(buffer, 1, 4096, src)) > 0) {
          fwrite(buffer, 1, size, new);
        }

        fclose(src);
        fclose(new);

        strcpy(temp_merge_cpath, ".beargit/");
        strcat(temp_merge_cpath, commit_id);
        strcat(temp_merge_cpath, "/");
        fprintf(stdout, "%s conflicted copy created\n", line5);
        strcpy(new_name, "");
        yn = 1;
      } else if (line3 == NULL && yn == 0) {
        strcat(temp_merge_cpath, line5);
        fs_cp(temp_merge_cpath, line5);
        strcpy(temp_merge_cpath, ".beargit/");
        strcat(temp_merge_cpath, commit_id);
        strcat(temp_merge_cpath, "/");
        fprintf(stdout, "%s added\n", line5);
      }
      yn = 0;
    }
    fclose(cindex);
  }

  fclose(mindex);
  return 0;
  
}
