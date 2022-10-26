# MandelbrotSet
<p>This is my personal mandelbrot set project. It's nothing special, I can only guarantee that it runs on my hardware on Directx 11.3. Other hardware might cause it to
behave differently, or may even prevent it from running altogether.<br></p>
<hr style="height:1px !important;">
<p>The program uses a Compute Shader and a Texture2D to supersample the Mandelbrot Set at 4x. It then uses a Pixel Shader to draw from this texture to the screen.<br></p>
<p>Currently, the program only renders the Mandelbrot set at the default window, however, I plan to implement capablility for smootly exploring the Mandelbrot set by using two textures (one for the compute shader and one for the pixel shader) and computing the Mandelbrot set around the screen. I can then render to one texture in the background as the pixel shader displays information from the other texture.</p>
<p>The project is on hold for now as I learn about Visual Basic .NET</p>
