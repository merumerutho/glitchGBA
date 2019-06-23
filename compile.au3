; Used to include graphics in the C program

#pragma compile(Console, true)

#include <File.au3>
#include <Process.au3>

$MAXSIZETILES = 64000
$MAXSIZEMAP = 4096

ConsoleWrite('Exporting graphics...'&@CRLF)

$Folder = "graphics"

$File = FileOpen("graphics.h", 10)

$List = _FileListToArray($Folder ,"*")

; For each element in the list
For $i=1 To $List[0]
   $command = "grit.exe graphics\" & $List[$i] & " -gB8 -mRtf -pu16 -ftc"
   _RunDos($command)
   $List[$i] = StringTrimRight($List[$i], 4)
   FileWrite($File, '#include "'&$List[$i]&'.h"'&@CRLF)
   ConsoleWrite('Exported .c/.h of'&$List[$i]&@CRLF)
Next

ConsoleWrite("Assembling graphics.h..."&@CRLF)

FileWrite($File, @CRLF)
; Write number of tiles as NTILES
FileWrite($File, "#define NTILES "& $List[0] & @CRLF&@CRLF)

; Write lengths of tiles in TilesLen
FileWrite($File, "const unsigned int TilesLen[] = {"&@CRLF)
For $i=1 To $List[0]
   FileWrite($File, $List[$i]&"TilesLen, "&@CRLF)
Next
FileWrite($File, "};"&@CRLF&@CRLF)

; Write lengths of maps in MapsLen
FileWrite($File, "const unsigned short MapsLen[] = {"&@CRLF)
For $i=1 To $List[0]
   FileWrite($File, $List[$i]&"MapLen, "&@CRLF)
Next
FileWrite($File, "};"&@CRLF&@CRLF)

; Write lengths of palettes in PalsLen
FileWrite($File, "const unsigned short PalsLen[] = {"&@CRLF)
For $i=1 To $List[0]
   FileWrite($File, $List[$i]&"PalLen, "&@CRLF)
Next
FileWrite($File, "};"&@CRLF&@CRLF)

; Re-read and re-write memory on graphics.h

; Write memory locations of tiles of included graphics
FileWrite($File, "const unsigned int tilesPointer[]["&$MAXSIZETILES&"] __attribute__((aligned(4))) = {"&@CRLF)
For $i=1 to $List[0]
   $buffer = FileOpen($List[$i]&".c")
   $k = 1
   $size = Null
   ConsoleWrite("Importing tiles of "&$List[$i]&"..."&@CRLF)
   Do
	 $k+=1
	 $line = FileReadLine($buffer, $k)
	 If StringRegExp( $line, "Tiles\[(\d+)\]" ) Then
		$size = StringRegExp( $line, "Tiles\[(\d+)\]", 1 )
	 EndIf
   Until $size <> Null
   $k+=1
   ; current position + total spaces + lines + offset
   $end = $k + Floor((Int($size[0])/8)/8) + (Int($size[0])/8)

   $string = ""
   Do
	  $string &= FileReadLine($buffer, $k)&@CRLF
	  $k+=1
   Until $k = $end
   FileWrite($File, $string & @CRLF &"},"& @CRLF)
   FileClose($buffer)
Next
FileWrite($File, "};"&@CRLF&@CRLF)

; Write memory locations of maps of included graphics
FileWrite($File, "const unsigned short mapsPointer[]["&$MAXSIZEMAP&"] __attribute__((aligned(4))) = {"&@CRLF)
For $i=1 to $List[0]
   $buffer = FileOpen($List[$i]&".c")
   $k = 1
   $size = Null
   ConsoleWrite("Importing map of "&$List[$i]&"..."&@CRLF)
   Do
	 $k+=1
	 $line = FileReadLine($buffer, $k)

	 If StringRegExp( $line, "Map\[(\d+)\]" ) Then
		$size = StringRegExp( $line, "Map\[(\d+)\]", 1 )
	 EndIf
   Until $size <> Null
   $k+=1
   ; current position + total spaces + lines + offset
   $end = $k + Floor((Int($size[0])/8)/8) + (Int($size[0])/8)

   $string = ""
   Do
	  $string &= FileReadLine($buffer, $k)&@CRLF
	  $k+=1
   Until $k = $end
   FileWrite($File, $string & @CRLF &"},"& @CRLF)
   FileClose($buffer)
Next
FileWrite($File, "};"&@CRLF&@CRLF)

; Write memory locations of palettes of included graphics
FileWrite($File, "const unsigned short palsPointer[][256] __attribute__((aligned(4))) = {"&@CRLF)
For $i=1 to $List[0]
   $buffer = FileOpen($List[$i]&".c")
   $k = 1
   $size = Null
   ConsoleWrite("Importing palette of "&$List[$i]&"..."&@CRLF)
   Do
	 $k+=1
	 $line = FileReadLine($buffer, $k)

	 If StringRegExp( $line, "Pal\[(\d+)\]" ) Then
		$size = StringRegExp( $line, "Pal\[(\d+)\]", 1 )
	 EndIf
   Until $size <> Null
   $k+=1
   ; current position + total spaces + lines + offset
   $end = $k + Floor((Int($size[0])/8)/8) + (Int($size[0])/8)
   $string = ""
   Do
	  $string &= FileReadLine($buffer, $k)&@CRLF
	  $k+=1
   Until $k = $end
   FileWrite($File, $string & @CRLF &"},"& @CRLF)
   FileClose($buffer)
Next
FileWrite($File, "};"&@CRLF&@CRLF)
FileClose($File)

; Done!

ConsoleWrite('Include file created'&@CRLF)
FileMove("*.h", "source\", 1)

For $i=1 to $List[0]
   FileDelete($List[$i]&".c")
Next

_RunDos("make")
