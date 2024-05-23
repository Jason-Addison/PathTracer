
Object castRay(vec3 rayOrigin, vec3 rayDirection, inout vec3 normal, inout vec3 intersectionPoint)
{
    vec3 colour = vec3(0.0, 0.0, 0.0);
    normal = vec3(0.0, 0.0, 0.0);
    float minDist = 1e6;
    Object intersectedObject = intersect(rayOrigin, rayDirection, minDist, normal);

    intersectionPoint = rayOrigin + rayDirection * minDist;
    return intersectedObject;
}

float schlick(float cosine, float ior)
{
    float R0 = pow((1.0 - ior) / (1.0 + ior), 2.0);
    return R0 + (1.0 - R0) * pow(1.0 - cosine, 5.0);
}

float diffuseBRDF(vec3 rayOut, vec3 n)
{
    return max(dot(n, rayOut), 0.0);
}

vec3 incomingLight(vec3 origin, vec3 normal)
{
    vec3 lightSample = vec3(0.0);
    vec3 diffuseVector = normalize(random3dVector(origin + float(frame)) * 2.0 - 1.0);

    for (int i = 0; i < OBJECT_COUNT; ++i)
    {
        Object object = objects[i];
        if (object.material.emission != vec3(0.0))
        {
            vec3 newPosition = vec3(0.0);
            vec3 newNormal = vec3(0.0);

            vec3 dir = normalize(diffuseVector + (objects[i].position - origin));

            if (castRay(origin, dir, newNormal, newPosition).id == i + 1)
            {
                lightSample += object.material.emission * diffuseBRDF(dir, normal);
            }
        }
    }
    return lightSample;
}

vec3 color(vec3 rayOrigin, vec3 rayDirection, float sampleIndex)
{
    vec3 attenuation = vec3(1.0);
    vec3 emission = vec3(0.0);

    float previousIOR = 1.0;

    for(int i = 0; i < MAX_BOUNCES; ++i)
    {
        vec3 intersectionPoint = vec3(0.0);
        vec3 normal = vec3(0.0);

        Object object = castRay(rayOrigin, rayDirection, normal, intersectionPoint);
        vec3 samplePoint = intersectionPoint;
        intersectionPoint += normal * EPSILON;
        

        Material material = object.material;

        vec3 incomingLight = incomingLight(intersectionPoint, normal);

        if(object.id != 0)
        {
            if(object.id == 2)
            {
                //Wood floor
                attenuation *= texture(textureMap, samplePoint.xz / 10).xyz;
            }
            else
            {
                attenuation *= material.color;
            }
            emission += material.emission + incomingLight;
        }
        else
        {
            emission += vec3(1.0, 1.0, 1.0);
            break;
        }

        vec3 diffuseVector = normalize(random3dVector(intersectionPoint + float(frame) + sampleIndex) * 2.0 - 1.0);

        float fresnel = schlick(dot(normal, -rayDirection), material.indexOfRefraction);
        float noise = noise(intersectionPoint.x + intersectionPoint.y + intersectionPoint.z + float(frame) + sampleIndex);

        vec3 reflectionVector = reflect(rayDirection, normal);

        if(noise > fresnel)
        {
            intersectionPoint -= 2.0 * normal * EPSILON;
            reflectionVector = refract(rayDirection, normal, previousIOR / material.indexOfRefraction);
        }

        rayDirection = mix(reflectionVector, diffuseVector, material.roughness);

        rayOrigin = intersectionPoint;

        if(material.indexOfRefraction > 0.0)
        {
            previousIOR = material.indexOfRefraction;
        }
        else
        {
            previousIOR = 1.0;
        }
    }
    return attenuation * emission;
}

void main()
{
    float f = float(frame);
    vec3 rayDirection = normalize(vec3(TexCoord.x * (resolution.x / resolution.y), TexCoord.y, -1.0));
    vec3 rayOrigin = cameraPosition;

    float cosX = cos(radians(cameraRotation.x));
    float sinX = sin(radians(cameraRotation.x));
    float cosY = cos(radians(cameraRotation.y));
    float sinY = sin(radians(cameraRotation.y));

    float tempY = rayDirection.y;
    rayDirection.y = tempY * cosX - rayDirection.z * sinX;
    rayDirection.z = tempY * sinX + rayDirection.z * cosX;

    float tempX = rayDirection.x;
    rayDirection.x = tempX * cosY + rayDirection.z * sinY;
    rayDirection.z = -tempX * sinY + rayDirection.z * cosY;

    rayDirection = normalize(rayDirection + random3dVector(rayOrigin + rayDirection * (f + 1.0)).xzy * 0.003);
    
    vec3 col = vec3(0.0);
    for(float n = 0.0; n < SAMPLES; ++n)
    {
       col += color(rayOrigin, rayDirection, n) / SAMPLES;
    }

    vec4 previousColor = texture(lastFrame, (TexCoord + 1)/2);
    FragColor = vec4((col + previousColor.xyz * (f - 1)) / f, 1.0);
}