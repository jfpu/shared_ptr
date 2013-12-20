/*=============================================================================
#     FileName: use_shared_ptr.cc
#         Desc: 
#       Author: Jeffrey Pu
#        Email: pujunying@gmail.com
#     HomePage: https://github.com/jfpu
#      Version: 0.0.1
#   LastChange: 2013-12-18 23:54:06
#      History:
=============================================================================*/

// g++ -o use_shared_ptr -std=c++0x use_shared_ptr.cc

#include <iostream>
#include <vector>
#include <cassert>
#include "shared_ptr.h"

int main()
{
    {
		#if 0
        int m = 0;
        jfpu::__shared_ptr<int> p(new int);
        jfpu::__shared_ptr<int> p2(p, &m);

        assert(&m == p2.get());
        assert(NULL != p2);
        assert(!!p2);
        assert(p2.use_count() == p.use_count());
        assert(!(p < p2) && !(p2 < p));
        jfpu::__shared_ptr<int volatile> p3;
        p2.reset(p3, 0);
        #endif
    }
    {
        int m = 0;
        jfpu::shared_ptr<int> p(new int);
        jfpu::shared_ptr<int> p2(p, &m);

        assert(&m == p2.get());
        assert(NULL != p2);
        assert(!!p2);
        assert(p2.use_count() == p.use_count());
        assert(!(p < p2) && !(p2 < p));
		#if 0
        jfpu::shared_ptr<int> p3;
        p2.reset(p3);
		assert(0 == p3.use_count());
		assert(1 == p2.use_count());
		#endif
        jfpu::shared_ptr<int volatile> p4;
        p2.reset(p4, 0);
		assert(0 == p4.use_count());
		assert(0 == p2.use_count());
	}
	{
		while(1)
		{
			std::vector<jfpu::shared_ptr<int> > vec;
			for(int i = 0; i < 1000000; ) {
				vec.push_back(jfpu::shared_ptr<int>(new int(++i)));
			}
			std::vector<jfpu::shared_ptr<int> >::iterator it = vec.begin();
			std::vector<jfpu::shared_ptr<int> >::iterator itEnd = vec.end();
			std::cout << "vec: " << std::ends;
			for(; it != itEnd; ++it) {
				std::cout << *(it->get()) << " ";
			}
			std::cout << std::endl;
		}
    }
    {
        // #if 0
        int i = 0;
        std::vector<jfpu::shared_ptr<int> > vec(10);
        std::vector<jfpu::shared_ptr<int> >::iterator it = vec.begin();
        std::vector<jfpu::shared_ptr<int> >::iterator itEnd = vec.end();
        std::cout << "vec: " << std::ends;
        for(; it != itEnd; ++it) {
            (*it) = jfpu::shared_ptr<int>(new int(++i));
            std::cout << *(it->get()) << " ";
        }
        std::cout << std::endl;
        // #endif
    }
    
    return 0;
}


