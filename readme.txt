This is a project area. Be nice to each other and do great things! 

Important : Do not delete this file from perforce server. 
(You will lose view of your home directory if there is no other file in your Home directory. Perforce default depot view does not show empty folders.)

Perforce documentation available here : https://drive.google.com/drive/folders/0BwMghofSTHdpRXkzYU9tMG9XbXM
(Login using ade-buas.nl Google Suite account)

Ask your Queries here: #perforce channel on slack

Submit your requests here: perforce_admin@ade-buas.nl

Managing your submits on Perforce:
Use Perforce to keep track of your source files for Projects only.
Please do not submit files that are not required for your project to build.
Unlike Google Drive, Perforce does not have infinite storage. It is currently limited to 10 TB.

Golden Tips:
* Please don't use Perforce as your personal dumping ground.
* Do not commit generated or temporary content. (This also includes Builds which can be generated using source files)
* Artists: Do not store multiple revisions of the same file (ie. My8KGrassTexture_01.pdf, My8KGrassTexture_02.pdf, My8KGrassTexture_03.pdf, etc...). Perforce keeps tracks of your file history so keep only My8KGrassTexture.pdf.

In case of any questions/doubts regarding your submits, feel free to post in #perforce channel on Slack, or send a mail to perforce_admin@ade-buas.nl or ask your team mates.

Use your Google drive to store your builds (or other cloud storage). Perforce should contain everything to make a build not the actual built game. 
Use labels to tag the files needed to recreate a release (https://www.perforce.com/perforce/r14.3/manuals/p4v/branches.labels.html)

Try to compress the videos, big images into lower resolution if possible.

Use Perforce Ignore File for your projects to manage submits to your server space:
(Perforce supports the P4IGNORE environment variable. This allows you to specify files and directories to ignore when using the commands that search for or add new files.
To use an ignore file, create a file in the root of your workspace and give it some meaningful name. 
The convention seems to be something like .ignore or .p4ignore or .txt, but anything will do (Use p4ignore.txt so that you can edit it with a simple double-click). 

Then fill it with your ignore rules. The following will ignore the the unwanted debris generated for your local machine: ("#" SUGGESTS ITS A COMMENT IN ignore file)


#******** Sample Content of ignore file ********************

# directories
#everything inside this directory is ignored for server operations
Generated_Dir_1
Generated_Dir_2\Sub-dir

# files
#All files with .bin and .obj extension will be ignored for server operations
*.bin
*.obj

#Override the ignore list to Exclude the necessary files/folders from ignore list
#To exclude files from Ignore List use : "!"
!Required.bin
#The above combination will ignore all .bin files except Required.bin

#******** Sample Content of ignore file ********************


After you have created this file, set the P4IGNORE environment variable to point to it. At the command line, type this:
p4 set P4IGNORE=C:\somepath\p4ignore.txt
Be sure to use an absolute path!

After doing this, any attempt to add files or directories that are in the ignore list will be rejected for submit/reconsile etc. 


Thanks for your time, Let me know in case of any other questions
Best wishes !!
Abhishek. (You can also reach me in this mail : biswas.a@buas.nl)

