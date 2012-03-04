#ifndef _COMMANDS_H_
#define _COMMANDS_H_

#include "hook.h"
#include "cd_cmd.h"
#include "cmd_prompt.h"
#include "cp_cmd.h"
#include "geterror_cmd.h"
#include "ls_cmd.h"
#include "md5sum_cmd.h"
#include "mkdir_cmd.h"
#include "more_cmd.h"
#include "pwd_cmd.h"
#include "mv_cmd.h"
//#include "cmd_mount.h"
#include "cmd_testsuite.h"
#include "fsinfo_cmd.h"
#include "hexview_cmd.h"
#include "mkfile_cmd.h"

#ifdef WIN32
#include "cmd_Windows/md5sum_win_cmd.h"
#else
#include "cmd_Linux/md5sum_lin_cmd.h"
#endif

#endif
