#pragma once

#include "can/core/types.h"

namespace can::sim::filter {

/**
 * A filtering callable
 */
class Filter {
  public:
    /**
     * Constructor
     * @param type how va1 and val2 are interpreted as a filter.
     * @param config what should happen if the filter matches
     * @param val1 filter value 1
     * @param val2 filter value 2
     */
    Filter(CanFilterType type, CanFilterConfig config, uint32_t val1,
           uint32_t val2)
        : type{type}, config{config}, val1{val1}, val2{val2} {}

    /**
     * Method called to apply fi
     * @param arbitration_id
     * @return True if message is to be accepted by this filter.
     */
    bool operator()(uint32_t arbitration_id) const {
        auto matches = false;
        switch (type) {
            case (CanFilterType::mask):
                matches = (val2 & arbitration_id) == val1;
                break;
            case (CanFilterType::range):
                matches = arbitration_id >= val1 && arbitration_id <= val2;
                break;
            case (CanFilterType::exact):
                matches = arbitration_id == val1 || arbitration_id == val2;
                break;
        }
        // Return true only if the filter matched and the config is not reject.
        return matches ? config != CanFilterConfig::reject : false;
    }

  private:
    CanFilterType type;
    CanFilterConfig config;
    uint32_t val1;
    uint32_t val2;
};

}  // namespace sim_filter