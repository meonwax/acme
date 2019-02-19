#/bin/bash
#
# Get release and create RC-File
#
function DEBUG()
{
    [ "$_DEBUG" == "on" ] &&  $@
}
function pause()
{
    read -p "Weiter mit Eingabe" $a
}
FILE="resource.rc"
RELEASE=`grep "define RELEASE"  ../version.h  | cut -f3`
RELEASE=${RELEASE//\"/}
DEBUG echo $RELEASE
VERSION=${RELEASE//./,},0
DEBUG echo $VERSION
CODENAME=`grep "define CODENAME"  ../version.h  | cut -f2`
DEBUG echo $CODENAME
FILEVERSION=\""$RELEASE ${CODENAME//\"/}"\"
DEBUG echo $FILEVERSION

cat << EndOfFile > $FILE
// Iconfile (64/32/16)
ID ICON "Logo.ico"

// Infos for windows
1 VERSIONINFO
FILEVERSION     $VERSION
PRODUCTVERSION  $VERSION
FILEFLAGSMASK   0x3fL
FILEFLAGS       0x0L
FILEOS          0x40004L
FILETYPE        0x2L
FILESUBTYPE     0x0L
BEGIN
  BLOCK "StringFileInfo"
  BEGIN
    BLOCK "040904E4"
    BEGIN
      VALUE "CompanyName", "Smørbrød Software"
      VALUE "FileDescription", "Acme crossassembler"
      VALUE "FileVersion", $FILEVERSION
      VALUE "InternalName", "ACME crossassembler"
      VALUE "LegalCopyright", "Copyright © 2018 Marco Baye"
      VALUE "OriginalFilename", "acme.exe"
      VALUE "ProductName", "ACME Crossassembler"
      VALUE "ProductVersion", $FILEVERSION
      VALUE "PorductLicence","GNU General Public License"
      VALUE "WindowsPort","Dirk Höpfner hoeppie@gmx.de"
    END
  END
  BLOCK "VarFileInfo"
  BEGIN
    VALUE "Translation", 0x409, 1252
  END
END
EndOfFile
