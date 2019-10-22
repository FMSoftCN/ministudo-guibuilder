/*
** This file is a part of miniStudio, which provides a WYSIWYG UI designer
** and an IDE for MiniGUI app developers.
**
** Copyright (C) 2010 ~ 2019, Beijing FMSoft Technologies Co., Ltd.
**
** This program is free software: you can redistribute it and/or modify
** it under the terms of the GNU General Public License as published by
** the Free Software Foundation, either version 3 of the License, or
** (at your option) any later version.
**
** This program is distributed in the hope that it will be useful,
** but WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
** GNU General Public License for more details.
**
** You should have received a copy of the GNU General Public License
** along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef MAPEX_H_
#define MAPEX_H_

#include <map>
template<typename _Ky, typename _Tp,typename _Compare = std::less<_Ky>, typename _Alloc = std::allocator<std::pair<const _Ky, _Tp> > >
class mapex : public std::map<_Ky, _Tp, _Compare, _Alloc>
{
public:
	typedef std::map<_Ky, _Tp, _Compare, _Alloc> map_type;
	typedef typename map_type::iterator iterator;
	_Tp at(const _Ky& key)
	{
		iterator it = map_type::find(key);
		if(it == map_type::end())
			return _Tp(0);
		return it->second;
	}
};

#endif /* MAPEX_H_ */
