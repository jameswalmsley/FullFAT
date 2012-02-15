/*****************************************************************************
 *  FullFAT - High Performance, Thread-Safe Embedded FAT File-System         *
 *  Copyright (C) 2009  James Walmsley (james@worm.me.uk)                    *
 *                                                                           *
 *  This program is free software: you can redistribute it and/or modify     *
 *  it under the terms of the GNU General Public License as published by     *
 *  the Free Software Foundation, either version 3 of the License, or        *
 *  (at your option) any later version.                                      *
 *                                                                           *
 *  This program is distributed in the hope that it will be useful,          *
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of           *
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the            *
 *  GNU General Public License for more details.                             *
 *                                                                           *
 *  You should have received a copy of the GNU General Public License        *
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.    *
 *                                                                           *
 *  IMPORTANT NOTICE:                                                        *
 *  =================                                                        *
 *  Alternative Licensing is available directly from the Copyright holder,   *
 *  (James Walmsley). For more information consult LICENSING.TXT to obtain   *
 *  a Commercial license.                                                    *
 *                                                                           *
 *  See RESTRICTIONS.TXT for extra restrictions on the use of FullFAT.       *
 *                                                                           *
 *  Removing the above notice is illegal and will invalidate this license.   *
 *****************************************************************************
 *  See http://worm.me.uk/fullfat for more information.                      *
 *  Or  http://fullfat.googlecode.com/ for latest releases and the wiki.     *
 *****************************************************************************/

using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Runtime.InteropServices;
using System.IO;

/*
int FFC_MountImage(char *szFilename, unsigned long ulBlockSize, unsigned long ulPartitionNumber);
int FFC_CleanupTerminal();
int FFC_InitTerminal();
int FFC_Exec(char *commandLine);
int FFC_UnmountImage(); 
*/

namespace DLLTestCS
{
    class Program
    {
        [DllImport("FullFAT32.dll")]
        public static extern int FFC_Exec(String commandLine);
        [DllImport("FullFAT32.dll")]
        public static extern int FFC_InitTerminal();
        [DllImport("FullFAT32.dll")]
        public static extern int FFC_CleanupTerminal();
        [DllImport("FullFAT32.dll")]
        public static extern int FFC_MountImage(String FileName, UInt32 ulBlockSize, UInt32 ulPartitionNumber);
        [DllImport("FullFAT32.dll")]
        public static extern int FFC_UnmountImage();

        static void Main(string[] args)
        {

            int Error;

            // Firstly initialise the terminal. (Initialises all the commands etc!)
            FFC_InitTerminal();

            // Then mount an image file!
            //Error = FFC_MountImage("\\\\.\\PHYSICALDRIVE1", 0, 0);    // Opening a physical drive!

            // Blocksize is most likely 512 bytes, and partition should be 0.
            Error = FFC_MountImage("c:\\myimage.img", 512, 0);

            /*if (Error != 0)
            {
                Console.Write("Could not mount image!\n");
                return;
            }*/
     
            // Execute commands to fill the image file!
            
            Error = FFC_Exec("ls");

            //Error = FFC_Exec("icp test.txt \\test.txt");

            //Error = FFC_Exec("ls");

            //Error = FFC_Exec("view test.txt");

            
            // Unmount the image to flush all changes to disk.
            
            Error = FFC_UnmountImage();

            // Cleanup the Terminal program from memory -- NOTE this doesn't really free memory but it will when I write a cleanup 
            // for FFTerm!! (Just call it anyway, for when I send you an update of FFTerm).
            Error = FFC_CleanupTerminal();

        }
    }
}
