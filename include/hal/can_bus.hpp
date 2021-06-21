#pragma once

#include <concepts>


template<class CAN, class MESSAGE>
concept CanBus = requires(CAN can, MESSAGE msg ) {
    {can.send(msg) };
};
