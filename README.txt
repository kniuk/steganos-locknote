Steganos LockNote - Self-modifying encrypted notepad
Copyright (C) 2006-2010 Steganos GmbH


Build Notes
-----------

Written by Robert Foertsch, 2010/02/16

As of version 1.0.5, LockNote builds with Microsoft Visual Studio 2008.
The old solution and project files for Microsoft Visual Studio .NET 2003
are included as well.

There is also initial support for the SCons build system, but you need
to manually modify the SConstruct file to get it to work.

The LockNote source has following dependencies:

* CryptoPP, available at http://www.cryptopp.com/.
  The source must reside in the cryptopp subdirectory, and a compiled libfile
  named 'cryptlib.lib' must reside in the root folder of the LockNote source.
  (Note that if you build a Debug version, you need a Debug build of 'cryptlib.lib'
  in the root folder)

* AESPHM, a module using the CryptoPP, available at
  http://www.denisbider.com/aesphm.zip
 
* WTL, available at http://wtl.sourceforge.net/.
  The WTL include files must reside in the wtl80 subdirectory.

The release build will compile with deprecation warnings in VS 2008 due to some
functions used in the code for which there are now replacements.


History
-------

* 1.0.5, 2010/02/16:
	- FIX: Find Dialog: Enter/Tab/Escape/Space/Ctrl-V keys work
	- FIX: Find Dialog: Message displayed if search string is not found
	- FIX: Find Dialog: F3 brings up Find Dialog or finds next result
	- NEW: Changed default font to Lucida Console to avoid 1/l - O/0 confusion
	- NEW: Font typeface can be selected (Arial/Courier New/Lucida Console/Tahoma/Verdana)
	- NEW: Font size can be selected (9/10/12/14)
	- NEW: LockNote saves window size, font size and font typeface
	
* 1.0.4, 2007/04/05:
	- NEW: Added Dutch translation
	- FIX: Path fix for cryptopp.lib in project

* 1.0.3, 2006/03/06:
	- NEW: Added Spanish translation (Castilian)

* 1.0.2, 2006/02/14:
	- NEW: Added French translation
	- NEW: Added 'Save As...' option

* 1.0.1, 2006/01/16:
	- FIX: Removed 30k character limit
	- NEW: Conversion displays a summary message box
	- FIX: Wrong filename displayed when conversion fails, fixed
	- NEW: Made conversion error message more comprehensible
	- FIX: Made case insensitive search work
	- FIX: Filenames are no longer being converted to lowercase
	- FIX: Document was touched even when closing without changes
	- NEW: Dragging files on program icon converts files as well

* 1.0, 2006/01/06: Initial release.


License
-------

This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA

