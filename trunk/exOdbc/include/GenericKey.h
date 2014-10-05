/*!
* \file GenericKey.h
* \author Elias Gerber <eg@zame.ch>
* \date 26.07.2014
* \brief Unused leftover from wxWidgets 2.8. Will be removed soon.
*
* For completion, here is the original wxWidgets header:
*
* / ///////////////////////////////////////////////////////////////////////////////<br>
* // Name:        dbkeyg.h<br>
* // Purpose:     Generic key support for wxDbTable<br>
* // Author:      Roger Gammans<br>
* // Modified by:<br>
* // Created:<br>
* // RCS-ID:      $Id: dbkeyg.h 29077 2004-09-10 12:56:07Z ABX $<br>
* // Copyright:   (c) 1999 The Computer Surgery (roger@computer-surgery.co.uk)<br>
* // Licence:     wxWindows licence<br>
* //<br>
* // NOTE : There is no CPP file to go along with this<br>
* //<br>
* ///////////////////////////////////////////////////////////////////////////////<br>
* // Branched From : gkey.h,v 1.3 2001/06/01 10:31:41<br>
* ///////////////////////////////////////////////////////////////////////////////<br>
*/

#pragma  once
#ifndef GENERICKEY_H
#define GENERICKEY_H

// Same component headers
// Other headers
// System headers

namespace exodbc
{
	/*!
	* \class GenericKey
	*
	* \brief Unused leftover from wxWidgets 2.8. Will probably be removed soon.
	*
	*/
	class GenericKey
	{
	public:
		GenericKey(void *blk, size_t sz)    { clone(blk, sz); }
		GenericKey(const GenericKey &ref)   { clone(ref.m_data, ref.m_sz); }
		~GenericKey()                       { free(m_data); }

		void *GetBlk() const { return m_data; }

	private:
		void clone(void *blk, size_t sz)
		{
			m_data = malloc(sz);
			memcpy(m_data, blk, sz);
			m_sz = sz;
		}

		void   *m_data;
		size_t  m_sz;
	};

}

#endif // GENERICKEY_H
