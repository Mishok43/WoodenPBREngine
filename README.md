<img src="https://github.com/Mishok43/WoodenPBREngine/blob/master/images/dino.PNG" width="256" height="256"><img src="https://github.com/Mishok43/WoodenPBREngine/blob/master/images/anisotropic.png" width="256" height="256"><img src="https://github.com/Mishok43/WoodenPBREngine/blob/master/images/mG-8P4TLfQU.jpg" width="256" height="256">  
<img src="https://github.com/Mishok43/WoodenPBREngine/blob/master/images/dielectric.png" width="256" height="256"><img src="https://github.com/Mishok43/WoodenPBREngine/blob/master/images/conductor.png" width="256" height="256"><img src="https://github.com/Mishok43/WoodenPBREngine/blob/master/images/StratisfiedSampler.PNG" width="256" height="256">   

Wooden PBR Engine is software ray-tracing researching engine for rendering 3D scenes based on C++17, SIMD Math Library, Data-Oriented ECS Design.  
<b>Technologies</b>:   
C++17, Data-Oriented ECS, Allocators, SIMD Math  

<b>Paper(RU)</b>:  
https://drive.google.com/file/d/1mX7miJLeN_xeZ3ocKnfiUIN57I690-i9/view?usp=sharing  

<b>Implemented features</b>:  
  - Basic geometry shapes meshes generator  
  - Exporter models data from some formats  
  - Area lights, Infinite area light  
  - Texturing  
  - Billinear, Anisotropic samplers  
  - Filter functions for reconstruction  
  - Uniform, Stratified sampler  
  - Uniform sampling different geometric surfaces  
  - Dielectric, conductor  fresnel  
  - BRDF, BSDF, Micro-surface distribution  
  - Linear Bounding Volume Hierarchy - hybrid building method based on surface area heuristic + fast morton code 3d volume separation  
  - Multiple Importance sampling  
  - Several materials (plastic, metal)  
  - Blackbody emitters  
  
<b>In progress</b>:  
  - Metropolis sampling  
  - Photon mapping  
  - Volume rendering  
  - Subsurface scattering  
  - Multilayer materials
  - Scene parser  
  
<b>Libraries</b>:  
  Data Driven ECS: https://github.com/Mishok43/WoodenECS  
  Math: https://github.com/Mishok43/WoodenMath  
  Allocators: https://github.com/Mishok43/WoodenAllocators  

<b>Based on</b>:   
  Physically Based Rendering from Theory To Implementation  
  Computer architecture a quantitative approach   

<h1> Features </h1>

<h2> Enviroment map </h2>   
<img src="https://github.com/Mishok43/WoodenPBREngine/blob/master/images/testpng.png" width="256" height="256"><img src="https://github.com/Mishok43/WoodenPBREngine/blob/master/images/ball0.png" width="256" height="256"><img src="https://github.com/Mishok43/WoodenPBREngine/blob/master/images/ball2.png" width="256" height="256">
