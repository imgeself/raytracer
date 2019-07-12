A path tracer that I build for learning computer graphics and math behind. 
I wrote some [blog](https://imgeself.github.io/) posts along with the implementation.

In this project I'm exploring "Handmade Hero" and [Semantic Compression](https://caseymuratori.com/blog_0015) mindset. 
So I'm not using fancy C++ features. (like templates, oop, std lib, etc.)

I'm using these resources as a guide for developing the raytracer:
 - [Handmade Ray](https://hero.handmade.network/episode/ray/)
 - [Peter Shirley's ray tracing books](https://github.com/petershirley)
 - [Aras's amazing blog posts](http://aras-p.info/blog/2018/03/28/Daily-Pathtracer-Part-0-Intro/)
 - [Scratchapixel](https://www.scratchapixel.com/index.php)

## Some features
 - Intersection with planes and spheres
 - Support for reflective, diffuse, emissive and dielectric materials
 - Multithreading
 - SIMD sphere intersection checking
 - Sampling
 - Antialiasing with sampling
 - GPU port (OpenGL/Compute Shaders)
 - No 3rd lib (I think it's a very good feature)
 
Here is an example image that generated with the raytracer:

![Render exapmle](https://raw.githubusercontent.com/imgeself/imgeself.github.io/master/assets/img/emit-8000spp.png)

## Roadmap
These are the things I will try to implement
 - Cornell box
 - Bounding Volume Hierarchies
 - 3D Model rendering
 - Texture mapping
 - Smarter sampling (importance sampling)
 - More light types
 - BRDF
 - Physically based camera system
 - Monte Carlo integration
