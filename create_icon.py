#!/usr/bin/env python3
"""
Create a simple icon for the 3DS Space Odyssey game
Shows a bright star within a bubble
"""

from PIL import Image, ImageDraw
import os

def create_star_bubble_icon():
    # Create a 48x48 image (3DS icon size)
    size = 48
    img = Image.new('RGBA', (size, size), (0, 0, 0, 0))  # Transparent background
    draw = ImageDraw.Draw(img)
    
    # Define colors
    bubble_color = (100, 150, 255, 200)  # Light blue bubble with transparency
    star_color = (255, 255, 100, 255)    # Bright yellow star
    star_center_color = (255, 255, 200, 255)   # Lighter yellow center
    outline_color = (50, 100, 200, 255)  # Darker blue outline
    
    # Draw the bubble (circle)
    bubble_center = (size // 2, size // 2)
    bubble_radius = 20
    
    # Bubble outline
    draw.ellipse([bubble_center[0] - bubble_radius - 1, bubble_center[1] - bubble_radius - 1,
                  bubble_center[0] + bubble_radius + 1, bubble_center[1] + bubble_radius + 1],
                 fill=outline_color)
    
    # Bubble fill
    draw.ellipse([bubble_center[0] - bubble_radius, bubble_center[1] - bubble_radius,
                  bubble_center[0] + bubble_radius, bubble_center[1] + bubble_radius],
                 fill=bubble_color)
    
    # Draw the star (5-pointed star)
    star_center = bubble_center
    star_outer_radius = 12
    star_inner_radius = 6
    
    # Calculate star points
    import math
    points = []
    for i in range(10):  # 5 outer points + 5 inner points
        angle = i * math.pi / 5 - math.pi / 2  # Start from top
        if i % 2 == 0:  # Outer points
            radius = star_outer_radius
        else:  # Inner points
            radius = star_inner_radius
        
        x = star_center[0] + radius * math.cos(angle)
        y = star_center[1] + radius * math.sin(angle)
        points.append((x, y))
    
    # Draw star outline
    draw.polygon(points, fill=outline_color)
    
    # Draw star fill
    draw.polygon(points, fill=star_color)
    
    # Add a bright center to the star
    center_radius = 3
    draw.ellipse([star_center[0] - center_radius, star_center[1] - center_radius,
                  star_center[0] + center_radius, star_center[1] + center_radius],
                 fill=star_center_color)
    
    # Add some sparkle effects around the bubble
    sparkle_positions = [
        (8, 8), (40, 8), (8, 40), (40, 40),
        (24, 4), (44, 24), (24, 44), (4, 24)
    ]
    
    for pos in sparkle_positions:
        # Small sparkle dots
        draw.ellipse([pos[0] - 1, pos[1] - 1, pos[0] + 1, pos[1] + 1], 
                     fill=(255, 255, 255, 180))
    
    return img

def main():
    # Create the icon
    icon = create_star_bubble_icon()
    
    # Save as PNG
    icon.save('icon.png', 'PNG')
    print("Icon created: icon.png")
    
    # Also create a larger version for preview
    large_icon = icon.resize((128, 128), Image.NEAREST)
    large_icon.save('icon_large.png', 'PNG')
    print("Large preview created: icon_large.png")

if __name__ == "__main__":
    main()
