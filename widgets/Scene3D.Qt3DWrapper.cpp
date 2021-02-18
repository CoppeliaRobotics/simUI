TYPE("transform", transform, Qt3DCore::QTransform,)
PARAM_FLOAT("scale",
    o->setScale(value);
)
PARAM_FLOAT("rotation-x",
    o->setRotationX(value);
)
PARAM_FLOAT("rotation-y",
    o->setRotationY(value);
)
PARAM_FLOAT("rotation-z",
    o->setRotationZ(value);
)
PARAM_VEC3("translation",
    o->setTranslation(vector);
)
PARAM_VEC3("scale3d",
    o->setScale3D(vector);
)
PARAM_VEC4("rotation",
    o->setRotation(quaternion);
)
ENDTYPE

TYPE("camera", camera, Qt3DRender::QCamera,)
PARAM_INT("projection-type",
    o->setProjectionType((Qt3DRender::QCameraLens::ProjectionType)value);
)
PARAM_INT("orthographic-projection",
    o->setProjectionType(Qt3DRender::QCameraLens::OrthographicProjection);
)
PARAM_INT("perspective-projection",
    o->setProjectionType(Qt3DRender::QCameraLens::PerspectiveProjection);
)
PARAM_INT("frustum-projection",
    o->setProjectionType(Qt3DRender::QCameraLens::FrustumProjection);
)
PARAM_FLOAT("aspect-ratio",
    o->setAspectRatio(value);
)
PARAM_FLOAT("bottom",
    o->setBottom(value);
)
PARAM_FLOAT("exposure",
    o->setExposure(value);
)
PARAM_FLOAT("far-plane",
    o->setFarPlane(value);
)
PARAM_FLOAT("field-of-view",
    o->setFieldOfView(value);
)
PARAM_FLOAT("left",
    o->setLeft(value);
)
PARAM_FLOAT("near-plane",
    o->setNearPlane(value);
)
PARAM_FLOAT("right",
    o->setRight(value);
)
PARAM_FLOAT("top",
    o->setTop(value);
)
PARAM_VEC3("position",
    o->setPosition(vector);
)
PARAM_VEC3("up-vector",
    o->setUpVector(vector);
)
PARAM_VEC3("view-center",
    o->setViewCenter(vector);
)
ENDTYPE

TYPE("camera-controller-first-person", camera_controller_first_person, Qt3DExtras::QFirstPersonCameraController,)
PARAM_INT("camera",
    o->setCamera(dynamic_cast<Qt3DRender::QCamera*>(nodeById(value)));
)
PARAM_FLOAT("linear-speed",
    o->setLinearSpeed(value);
)
PARAM_FLOAT("look-speed",
    o->setLookSpeed(value);
)
PARAM_FLOAT("acceleration",
    o->setAcceleration(value);
)
PARAM_FLOAT("deceleration",
    o->setDeceleration(value);
)
ENDTYPE

TYPE("camera-controller-orbit", camera_controller_orbit, Qt3DExtras::QOrbitCameraController,)
PARAM_INT("camera",
    o->setCamera(dynamic_cast<Qt3DRender::QCamera*>(nodeById(value)));
)
PARAM_FLOAT("linear-speed",
    o->setLinearSpeed(value);
)
PARAM_FLOAT("look-speed",
    o->setLookSpeed(value);
)
PARAM_FLOAT("zoom-in-limit",
    o->setZoomInLimit(value);
)
ENDTYPE

TYPE("light-point", light_point, Qt3DRender::QPointLight,)
PARAM_FLOAT("intensity",
    o->setIntensity(value);
)
PARAM_FLOAT("constant-attenuation",
    o->setConstantAttenuation(value);
)
PARAM_FLOAT("linear-attenuation",
    o->setLinearAttenuation(value);
)
PARAM_FLOAT("quadratic-attenuation",
    o->setQuadraticAttenuation(value);
)
PARAM_VEC3("color",
    o->setColor(color);
)
ENDTYPE

TYPE("light-directional", light_directional, Qt3DRender::QDirectionalLight,)
PARAM_FLOAT("intensity",
    o->setIntensity(value);
)
PARAM_VEC3("color",
    o->setColor(color);
)
PARAM_VEC3("world-direction",
    o->setWorldDirection(vector);
)
ENDTYPE

TYPE("light-spot", light_spot, Qt3DRender::QSpotLight,)
PARAM_FLOAT("intensity",
    o->setIntensity(value);
)
PARAM_FLOAT("constant-attenuation",
    o->setConstantAttenuation(value);
)
PARAM_FLOAT("linear-attenuation",
    o->setLinearAttenuation(value);
)
PARAM_FLOAT("quadratic-attenuation",
    o->setQuadraticAttenuation(value);
)
PARAM_FLOAT("cutoff-angle",
    o->setCutOffAngle(value);
)
PARAM_VEC3("color",
    o->setColor(color);
)
PARAM_VEC3("local-direction",
    o->setLocalDirection(vector);
)
ENDTYPE

TYPE("mesh-cuboid", mesh_cuboid, Qt3DExtras::QCuboidMesh,)
PARAM_FLOAT("x-extent",
    o->setXExtent(value);
)
PARAM_FLOAT("y-extent",
    o->setYExtent(value);
)
PARAM_FLOAT("z-extent",
    o->setZExtent(value);
)
ENDTYPE

TYPE("mesh-torus", mesh_torus, Qt3DExtras::QTorusMesh,)
PARAM_FLOAT("radius",
    o->setRadius(value);
)
PARAM_FLOAT("minor-radius",
    o->setMinorRadius(value);
)
PARAM_INT("rings",
    o->setRings(value);
)
PARAM_INT("slices",
    o->setSlices(value);
)
ENDTYPE

TYPE("mesh-cone", mesh_cone, Qt3DExtras::QConeMesh,)
PARAM_FLOAT("top-radius",
    o->setTopRadius(value);
)
PARAM_FLOAT("bottom-radius",
    o->setBottomRadius(value);
)
PARAM_FLOAT("length",
    o->setLength(value);
)
PARAM_INT("rings",
    o->setRings(value);
)
PARAM_INT("slices",
    o->setSlices(value);
)
ENDTYPE

TYPE("mesh-cylinder", mesh_cylinder, Qt3DExtras::QCylinderMesh,)
PARAM_FLOAT("radius",
    o->setRadius(value);
)
PARAM_FLOAT("length",
    o->setLength(value);
)
PARAM_INT("rings",
    o->setRings(value);
)
PARAM_INT("slices",
    o->setSlices(value);
)
ENDTYPE

TYPE("mesh-plane", mesh_plane, Qt3DExtras::QPlaneMesh,)
PARAM_FLOAT("width",
    o->setWidth(value);
)
PARAM_FLOAT("height",
    o->setHeight(value);
)
ENDTYPE

TYPE("mesh-sphere", mesh_sphere, Qt3DExtras::QSphereMesh,)
PARAM_FLOAT("radius",
    o->setRadius(value);
)
PARAM_INT("rings",
    o->setRings(value);
)
PARAM_INT("slices",
    o->setSlices(value);
)
ENDTYPE

TYPE("mesh", mesh, Qt3DRender::QMesh,)
PARAM_STRING("source",
    o->setSource(url);
)
ENDTYPE

TYPE("material-phong", material_phong, Qt3DExtras::QPhongMaterial,)
PARAM_FLOAT("shininess",
    o->setShininess(value);
)
PARAM_VEC3("ambient",
    o->setAmbient(color);
)
PARAM_VEC3("diffuse",
    o->setDiffuse(color);
)
PARAM_VEC3("specular",
    o->setSpecular(color);
)
ENDTYPE

TYPE("material-gooch", material_gooch, Qt3DExtras::QGoochMaterial,)
PARAM_FLOAT("alpha",
    o->setAlpha(value);
)
PARAM_FLOAT("beta",
    o->setBeta(value);
)
PARAM_FLOAT("shininess",
    o->setShininess(value);
)
PARAM_VEC3("diffuse",
    o->setDiffuse(color);
)
PARAM_VEC3("specular",
    o->setSpecular(color);
)
PARAM_VEC3("cool-color",
    o->setCool(color);
)
PARAM_VEC3("warm-color",
    o->setWarm(color);
)
ENDTYPE

TYPE("material-texture", material_texture, Qt3DExtras::QTextureMaterial,)
PARAM_INT("texture",
    o->setTexture(dynamic_cast<Qt3DRender::QAbstractTexture*>(nodeById(value)));
)
PARAM_VEC2("texture-offset",
    o->setTextureOffset(vector);
)
ENDTYPE

TYPE("material-diffuse-map", material_diffuse_map, Qt3DExtras::QDiffuseMapMaterial,)
PARAM_FLOAT("shininess",
    o->setShininess(value);
)
PARAM_VEC3("ambient",
    o->setAmbient(color);
)
PARAM_INT("diffuse-texture",
    o->setDiffuse(dynamic_cast<Qt3DRender::QAbstractTexture*>(nodeById(value)));
)
PARAM_VEC3("specular",
    o->setSpecular(color);
)
PARAM_FLOAT("texture-scale",
    o->setTextureScale(value);
)
ENDTYPE

TYPE("texture2d", texture2d, Qt3DRender::QTexture2D,)
PARAM_INT("texture-image",
    o->addTextureImage(dynamic_cast<Qt3DRender::QAbstractTextureImage*>(nodeById(value)));
)
ENDTYPE

TYPE("texture-image", texture_image, Qt3DRender::QTextureImage,)
PARAM_STRING("source",
    o->setSource(url);
)
PARAM_INT("mirrored",
    o->setMirrored(value != 0);
)
ENDTYPE

#define object_picker_postinit QObject::connect(o, &Qt3DRender::QObjectPicker::clicked, UI::getInstance(), &UI::onScene3DObjectClicked);
TYPE("object-picker", object_picker, Qt3DRender::QObjectPicker, object_picker_postinit)
ENDTYPE

TYPE("entity", entity, Qt3DCore::QEntity,)
ENDTYPE

