#ifndef PLANT_H
#define PLANT_H

#include "entity.h"

class Plant : public Entity {
protected:
    int cost;

public:
    Plant(int x, int y,int health,int cost, std::string name,int ticks,int damage);

    virtual void action(Grid& grid) = 0;

    void update(Grid& grid) override;

    int getCost() const ;
};


#endif
