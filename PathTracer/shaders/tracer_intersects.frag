

float intersectSphere(vec3 rayOrigin, vec3 rayDirection, Object sphere, inout vec3 normal)
{
    vec3 centerToRay = rayOrigin - sphere.position;

    float b = dot(centerToRay, rayDirection) * 2.0;
    float c = dot(centerToRay, centerToRay) - sphere.scale.x * sphere.scale.x;
    float h = b * b - c * 4.0;
    float minDist = 0;
    if(h < 0.0)
    {
        //No intersection
        minDist = -1.0;
    }
    else
    {
        minDist = (-b - sqrt(h)) / 2.0;
    }

    normal = (centerToRay + rayDirection * minDist) / sphere.scale.x;
    return minDist;
}

float intersectCuboid(vec3 rayOrigin, vec3 rayDirection, Object cuboid, inout vec3 normal)
{
    vec3 bmin = cuboid.position - cuboid.scale.xyz / 2.0;
    vec3 bmax = cuboid.position + cuboid.scale.xyz / 2.0;
    
    vec3 minDist3 = (bmin - rayOrigin) / rayDirection;
    vec3 maxDist3 = (bmax - rayOrigin) / rayDirection;
    
    if (minDist3.x > maxDist3.x) swap(minDist3.x, maxDist3.x);
    if (minDist3.y > maxDist3.y) swap(minDist3.y, maxDist3.y);
    if (minDist3.z > maxDist3.z) swap(minDist3.z, maxDist3.z);
    
    float minDist = max3(minDist3);
    float maxDist = min3(maxDist3);
    float dist = min(minDist, maxDist);
    
    if (minDist > min3(maxDist3) || max3(minDist3) > maxDist)
    {
        dist = -1.0;
    }

    vec3 p = rayOrigin + rayDirection * dist;
    vec3 sn = 2.0 * (p - cuboid.position) / cuboid.scale.xyz;
    normal = vec3(0.0);
    
    if (sn.x > 1.0 - EPSILON * 2.0) normal = vec3(1.0, 0.0, 0.0);
    if (sn.x < EPSILON * 2.0 - 1.0) normal = vec3(-1.0, 0.0, 0.0);
    if (sn.y > 1.0 - EPSILON * 2.0) normal = vec3(0.0, 1.0, 0.0);
    if (sn.y < EPSILON * 2.0 - 1.0) normal = vec3(0.0, -1.0, 0.0);
    if (sn.z > 1.0 - EPSILON * 2.0) normal = vec3(0.0, 0.0, 1.0);
    if (sn.z < EPSILON * 2.0 - 1.0) normal = vec3(0.0, 0.0, -1.0);
    
    return dist;
}

Object intersect(vec3 rayOrigin, vec3 rayDirection, inout float minDist, inout vec3 normal)
{
    minDist = 1e6;
    int index = 0;
    for(int i = 0; i < OBJECT_COUNT; ++i)
    {
        float dist = 1e6;
        vec3 intersectNormal = vec3(0.0, 0.0, 0.0);

        if(objects[i].type == SPHERE)
        {
            dist = intersectSphere(rayOrigin, rayDirection, objects[i], intersectNormal);
        }
        else if(objects[i].type == CUBOID)
        {
            dist = intersectCuboid(rayOrigin, rayDirection, objects[i], intersectNormal);
        }

        if(dist < minDist && dist > 0.0)
        {
            minDist = dist;
            normal = normalize(intersectNormal);
            index = i;
        }
    }
    return objects[index];
    if(minDist < 1e6)
    {
        return objects[index];
    }
    return SKY;
}