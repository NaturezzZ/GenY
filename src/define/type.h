//
// Created by Naiqian on 2021/4/24.
//

#ifndef GENY_TYPE_H
#define GENY_TYPE_H
#include <bits/stdc++.h>
class BaseType;
using TypePtr = std::shared_ptr<BaseType>;
using TypePtrList = std::vector<TypePtr>;

class BaseType {
public:
    BaseType();
};
#endif //GENY_TYPE_H
