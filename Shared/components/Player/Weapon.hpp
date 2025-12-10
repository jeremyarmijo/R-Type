#pragma once
#include "components/Player/Projectile.hpp"

struct Weapon {
    Projectile projectileType = Projectile();
    float fireRate; // projectiles per second

    bool isAutomatic; // true for automatic, false for semi-automatic
    int maxAmmo = -1; // -1 for infinite ammo
    int magazineSize = -1; // -1 for no magazine
    float reloadTime = -1.0f; // time to reload in seconds & -1 for no reload


    bool isBurst;
    size_t burstCount = 3; // number of shots in a burst
    float burstInterval = 0.1f; // time between shots in a burst

    float timeSinceLastShot;
    int currentAmmo;

    explicit Weapon(float rate = 1.0f, bool automatic = false, int ammo = -1, int magSize = -1, float reload = -1.0f,
                    bool burst = false, size_t bCount = 3, float bInterval = 0.1f)
        : fireRate(rate), isAutomatic(automatic), maxAmmo(ammo), magazineSize(magSize),
          reloadTime(reload), isBurst(burst), burstCount(bCount), burstInterval(bInterval),
          timeSinceLastShot(0.0f), currentAmmo(magSize) {}
};