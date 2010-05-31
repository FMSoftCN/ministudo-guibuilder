/*
 * mapex.h
 *
 *  Created on: 2009-4-2
 *      Author: dongjunjie
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
