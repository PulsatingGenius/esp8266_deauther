/*
   Copyright (c) 2020 Stefan Kremser
   This software is licensed under the MIT License. See the license file for details.
   Source: github.com/spacehuhn/esp8266_deauther
 */

#pragma once

#include <stdint.h>
#include <stddef.h>

class TargetList;
class Target;

typedef struct target_t target_t;

class Target {
    private:
        target_t* ptr;

    public:
        Target(target_t* ptr);

        uint8_t* from() const;
        uint8_t* to() const;
        uint8_t ch() const;

        bool operator==(const Target& t) const;
};

class TargetList {
    private:
        target_t* list_begin = NULL;
        target_t* list_end   = NULL;

        int list_size = 0;

        target_t* h = NULL;

    public:
        ~TargetList();

        void move(TargetList& t);

        void push(const uint8_t* from, const uint8_t* to, const uint8_t ch);

        Target get(int i);

        void begin();
        Target iterate();

        bool available();
        int size();
};