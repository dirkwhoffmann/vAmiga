//
//  vastd.cpp
//  vAmiga
//
//  Created by Dirk Hoffmann on 13.01.19.
//  Copyright © 2019 Dirk Hoffmann. All rights reserved.
//

#include <stdio.h>



bool releaseBuild()
{
#ifdef NDEBUG
    return true;
#else
    return false;
#endif
}
