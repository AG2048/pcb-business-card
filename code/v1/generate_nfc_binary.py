# This script generates the binary file that can be written to the NFC tag to be displayed.

# Input: list of images / gifs, and associated colour / timing data.

# Output: hex binary file for NFC tag

import os
from PIL import Image, ImageSequence
import numpy as np
import matplotlib.pyplot as plt

# LED matrix configuration
LED_ROWS = 6
LED_COLS = 12
LED_COUNT = LED_ROWS * LED_COLS

def resize_image(image, target_width=LED_COLS, target_height=LED_ROWS):
    """
    Resize image to target dimensions using high-quality resampling.
    """
    return image.resize((target_width, target_height), Image.Resampling.LANCZOS)

def convert_to_4bit_color(image):
    """
    Convert image colors to 4-bit per channel (0-15 range).
    Returns a numpy array with shape (height, width, 3) containing RGB values 0-15.
    """
    # Convert to RGB if not already
    if image.mode != 'RGB':
        image = image.convert('RGB')
    
    # Convert to numpy array
    img_array = np.array(image)
    
    # Convert from 8-bit (0-255) to 4-bit (0-15)
    # Using bit shifting for precise conversion
    img_4bit = img_array >> 4  # Shift right by 4 bits to get upper 4 bits
    
    return img_4bit

def display_frame(frame, title="Frame Preview"):
    """
    Display a 6x12 frame using matplotlib.
    Shows the frame as it will appear on the LED matrix.
    """
    try:
        # Convert 4-bit values back to 8-bit for display (multiply by 17 to map 0-15 to 0-255)
        display_frame = (frame * 17).astype(np.uint8)
        
        plt.figure(figsize=(8, 4))
        plt.imshow(display_frame, interpolation='nearest')
        plt.title(f"{title} ({LED_COLS}x{LED_ROWS} pixels)")
        plt.axis('off')
        
        # Add grid to show individual pixels
        for i in range(LED_ROWS + 1):
            plt.axhline(y=i-0.5, color='white', linewidth=0.5, alpha=0.3)
        for j in range(LED_COLS + 1):
            plt.axvline(x=j-0.5, color='white', linewidth=0.5, alpha=0.3)
        
        plt.tight_layout()
        plt.show()
        
        # Also print color values for debugging
        print(f"\nFrame color values (4-bit, 0-15 range):")
        for y in range(LED_ROWS - 1, -1, -1):
            row_str = ""
            for x in range(LED_COLS):
                r, g, b = frame[LED_ROWS-y-1, x]
                row_str += f"({r:2},{g:2},{b:2}) "
            print(f"Row {y}: {row_str}")
            
    except ImportError:
        print("Matplotlib not available. Showing text representation:")
        print(f"\n{title} ({LED_COLS}x{LED_ROWS} pixels):")
        for y in range(LED_ROWS):
            row_str = ""
            for x in range(LED_COLS):
                r, g, b = frame[y, x]
                # Simple text representation: show brightest channel
                max_val = max(r, g, b)
                if max_val > 8:
                    if r == max_val: row_str += "R"
                    elif g == max_val: row_str += "G"
                    else: row_str += "B"
                else:
                    row_str += "."
            print(f"Row {y}: {row_str}")
    except Exception as e:
        print(f"Error displaying frame: {e}")

def process_single_image(image_path):
    """
    Process a single image: resize to 6x12 and convert to 4-bit color.
    Returns numpy array with shape (6, 12, 3) containing RGB values 0-15.
    """
    try:
        with Image.open(image_path) as img:
            # Resize image
            resized = resize_image(img)
            
            # Convert to 4-bit color
            color_4bit = convert_to_4bit_color(resized)
            
            return color_4bit
    except Exception as e:
        print(f"Error processing image {image_path}: {e}")
        return None

def process_gif_frames(gif_path):
    """
    Process all frames of a GIF: resize each frame to 6x12 and convert to 4-bit color.
    Returns list of numpy arrays, each with shape (6, 12, 3) containing RGB values 0-15.
    """
    frames = []
    try:
        with Image.open(gif_path) as gif:
            for frame in ImageSequence.Iterator(gif):
                # Convert frame to RGB (GIFs can be in palette mode)
                frame_rgb = frame.convert('RGB')
                
                # Resize frame
                resized = resize_image(frame_rgb)
                
                # Convert to 4-bit color
                color_4bit = convert_to_4bit_color(resized)
                
                frames.append(color_4bit)
                
        return frames
    except Exception as e:
        print(f"Error processing GIF {gif_path}: {e}")
        return []

def frame_to_grb444_bytes(frame, show_preview=True):
    """
    Convert a 6x12x3 numpy array (RGB 0-15) to GRB444 byte format.
    Each pixel uses 1.5 bytes (12 bits total: 4 bits each for G, R, B).
    Returns bytearray suitable for the LED matrix.
    """
    # Display the frame before converting
    if show_preview:
        display_frame(frame, "Frame Preview")
    
    height, width, channels = frame.shape
    pixel_array_size = (LED_COUNT * 3) // 2  # 1.5 bytes per pixel
    pixel_data = bytearray(pixel_array_size)
    
    for y in range(height):
        for x in range(width):
            # Calculate pixel index considering zigzag pattern
            # Even rows: left to right, odd rows: right to left
            if y % 2 == 0:
                pixel_index = y * width + x
            else:
                pixel_index = y * width + (width - 1 - x)
            
            # Extract RGB values (0-15)
            r, g, b = frame[height - 1 - y, x]

            # Pack into GRB444 format
            byte_index = (pixel_index * 3) // 2
            
            if pixel_index % 2 == 0:
                # Even pixel: G and R in first byte, B in upper 4 bits of second byte
                pixel_data[byte_index] = (g << 4) | r
                if byte_index + 1 < len(pixel_data):
                    pixel_data[byte_index + 1] = (pixel_data[byte_index + 1] & 0x0F) | (b << 4)
            else:
                # Odd pixel: G in lower 4 bits of first byte, R and B in second byte
                pixel_data[byte_index] = (pixel_data[byte_index] & 0xF0) | g
                if byte_index + 1 < len(pixel_data):
                    pixel_data[byte_index + 1] = (r << 4) | b
    
    return pixel_data

# Example usage functions
def process_image_file(file_path, show_preview=True):
    """
    Process an image file and return the frame data.
    """
    if file_path.lower().endswith('.gif'):
        frames = process_gif_frames(file_path)
        processed_frames = []
        for i, frame in enumerate(frames):
            if show_preview:
                display_frame(frame, f"GIF Frame {i+1}/{len(frames)}")
            processed_frames.append(frame_to_grb444_bytes(frame, show_preview=False))
        return processed_frames
    else:
        frame = process_single_image(file_path)
        if frame is not None:
            return [frame_to_grb444_bytes(frame, show_preview=show_preview)]
        return []


def generate_bin_file(frame_data, initial_delay, output_path):
    """
    Generate the hex file, with header 4 bytes, and additional 4 bytes per frame.
    """
    num_frames = len(frame_data)
    output_data = [(num_frames & 0xFF00) >> 8, num_frames & 0x00FF, (initial_delay & 0xFF00) >> 8, initial_delay & 0x00FF]

    for frame in frame_data[:-1]:
        frame_colour_mode = 0b000
        frame_transition_delay = 512
        frame_duration = 512
        output_data.append([((frame_colour_mode & 0b111) << 5) | (frame_transition_delay >> 8), frame_transition_delay & 0xFF, (frame_duration >> 8) & 0xFF, frame_duration & 0xFF])
        output_data.append(frame)
    frame = frame_data[-1]  # Last frame
    frame_colour_mode = 0b000
    frame_transition_delay = 1024
    frame_duration = 1024
    output_data.append([((frame_colour_mode & 0b111) << 5) | (frame_transition_delay >> 8), frame_transition_delay & 0xFF, (frame_duration >> 8) & 0xFF, frame_duration & 0xFF])
    output_data.append(frame)
    
    # Write to .bin file
    with open(output_path, 'wb') as f:
        for data in output_data:
            if isinstance(data, bytearray):
                f.write(data)
            elif isinstance(data, list):
                f.write(bytearray(data))
            else:
                # convert data from int to bytes. Only take lower 8 bits
                f.write(bytearray([data & 0xFF]))
    
    # Check file size and alert if too large for NFC tag
    file_size = os.path.getsize(output_path)
    max_nfc_size = 8192 - 32  # 8KB NFC tag minus 32 bytes reserved
    
    print(f"Bin file generated: {output_path}")
    print(f"File size: {file_size} bytes")
    
    if file_size > max_nfc_size:
        print(f"⚠️  WARNING: File size ({file_size} bytes) exceeds NFC tag capacity!")
        print(f"   Maximum allowed: {max_nfc_size} bytes (8192 - 32 reserved)")
        print(f"   Excess: {file_size - max_nfc_size} bytes")
        print(f"   Consider reducing frames or image complexity.")
    else:
        remaining_space = max_nfc_size - file_size
        print(f"✓ File fits in NFC tag. Remaining space: {remaining_space} bytes")


def print_bin_file(file_path, bytes_per_line=4):
    """
    Print a bin file in formatted chunks.
    
    Args:
        file_path: Path to the .bin file
        bytes_per_line: Number of bytes to display per line (default: 4)
    """
    try:
        with open(file_path, 'rb') as f:
            data = f.read()
        
        if not data:
            print(f"File {file_path} is empty")
            return
        
        print(f"=== Bin File: {file_path} ===")
        print(f"Total size: {len(data)} bytes")
        print("Offset   | Hex Values           | ASCII")
        print("-" * 45)
        
        offset = 0
        while offset < len(data):
            # Get the next chunk of bytes
            chunk = data[offset:offset + bytes_per_line]
            
            # Format hex values
            bin_str = ' '.join(f'{byte:02X}' for byte in chunk)
            
            # Pad hex string to consistent width
            bin_str = bin_str.ljust(bytes_per_line * 3 - 1)
            
            # Convert to ASCII (printable chars only)
            ascii_str = ''.join(chr(byte) if 32 <= byte <= 126 else '.' for byte in chunk)
            
            # Print the line
            print(f"{offset:08X} | {bin_str} | {ascii_str}")
            
            offset += bytes_per_line
        
        print("-" * 45)
        print(f"End of file ({len(data)} bytes total)")
        
    except FileNotFoundError:
        print(f"Error: File '{file_path}' not found")
    except Exception as e:
        print(f"Error reading file '{file_path}': {e}")


def analyze_nfc_bin_file(file_path):
    """
    Analyze and decode NFC hex file format according to your Arduino code structure.
    """
    try:
        with open(file_path, 'rb') as f:
            data = f.read()
        
        if len(data) < 4:
            print("File too small to contain valid header")
            return
        
        print(f"=== NFC File Analysis: {file_path} ===")
        print(f"Total file size: {len(data)} bytes\n")
        
        # Parse header (first 4 bytes)
        num_frames = (data[0] << 8) | data[1]
        initial_delay = (data[2] << 8) | data[3]
        
        print("Header (4 bytes):")
        print(f"  Bytes 0-1: Number of frames = {num_frames}")
        print(f"  Bytes 2-3: Initial delay = {initial_delay} ms")
        print()
        
        offset = 4
        frame_num = 0
        
        while offset < len(data) and frame_num < num_frames:
            print(f"Frame {frame_num + 1}:")
            
            if offset + 4 > len(data):
                print("  Error: Incomplete frame header")
                break
            
            # Parse frame header (4 bytes)
            color_mode = (data[offset] >> 5) & 0x07
            transition_time = ((data[offset] & 0x1F) << 8) | data[offset + 1]
            frame_duration = (data[offset + 2] << 8) | data[offset + 3]
            
            print(f"  Frame header (bytes {offset}-{offset+3}):")
            # Print color mode with description
            color_mode_desc = {
              0b000: "GRB444",
              0b001: "Solid Red",
              0b010: "Solid Green",
              0b011: "Solid Blue",
              0b100: "Solid White",
              0b101: "Solid Black",
              0b110: "Solid Yellow",
              0b111: "Reserved"
            }.get(color_mode, "Unknown")
            print(f"    Color mode: {color_mode:03b} ({color_mode_desc})")
            print(f"    Transition time: {transition_time} ms")
            print(f"    Frame duration: {frame_duration} ms")
            
            offset += 4
            
            # Calculate expected frame data size
            if color_mode == 0:
                # GRB444 mode: 1.5 bytes per pixel
                frame_data_size = (LED_COUNT * 3) // 2
            else:
                # Solid color mode: 1 bit per pixel
                frame_data_size = LED_COUNT // 8
            
            print(f"    Frame data size: {frame_data_size} bytes")
            
            if offset + frame_data_size > len(data):
                print(f"    Error: Incomplete frame data (expected {frame_data_size} bytes)")
                break
            
            # Show first few bytes of frame data
            frame_data = data[offset:offset + min(16, frame_data_size)]
            bin_preview = ' '.join(f'{byte:02X}' for byte in frame_data)
            if frame_data_size > 16:
                bin_preview += " ..."
            print(f"    Frame data preview: {bin_preview}")
            
            offset += frame_data_size
            frame_num += 1
            print()
        
        if frame_num != num_frames:
            print(f"Warning: Expected {num_frames} frames, but only found {frame_num}")
        
        remaining_bytes = len(data) - offset
        if remaining_bytes > 0:
            print(f"Note: {remaining_bytes} extra bytes at end of file")
            
    except Exception as e:
        print(f"Error analyzing file: {e}")


def print_bin_continuous(file_path):
    """
    Print the entire hex file content as a continuous string of hex values.
    Format: FFAABB00ABC (no spaces, no line breaks)
    """
    try:
        with open(file_path, 'rb') as f:
            data = f.read()
        
        if not data:
            print(f"File {file_path} is empty")
            return
        
        print(f"=== Continuous Hex: {file_path} ===")
        print(f"File size: {len(data)} bytes")
        print("Hex content:")
        
        # Convert all bytes to uppercase hex without spaces
        bin_string = ''.join(f'{byte:02X}' for byte in data)
        print(bin_string)
        
        print(f"\nTotal hex characters: {len(bin_string)}")
        
    except FileNotFoundError:
        print(f"Error: File '{file_path}' not found")
    except Exception as e:
        print(f"Error reading file '{file_path}': {e}")



# Example usage
if __name__ == "__main__":
    # # Example: Process a single image
    # image_path = "example.png"
    # if os.path.exists(image_path):
    #     frame_data = process_image_file(image_path)
    #     print(f"Processed {len(frame_data)} frame(s) from {image_path}")
    #     if frame_data:
    #         print(f"First frame size: {len(frame_data[0])} bytes")
    
    # # Example: Process a GIF
    # gif_path = "animation.gif"
    # if os.path.exists(gif_path):
    #     frame_data = process_image_file(gif_path)
    #     print(f"Processed {len(frame_data)} frame(s) from {gif_path}")
    #     for i, frame in enumerate(frame_data):
    #         print(f"Frame {i}: {len(frame)} bytes")
    
    # # Example: Create a test pattern
    # test_frame = np.zeros((LED_ROWS, LED_COLS, 3), dtype=np.uint8)
    # # Create a red border
    # test_frame[0, :] = [15, 0, 0]  # Top row: red
    # test_frame[-1, :] = [15, 0, 0]  # Bottom row: red
    # test_frame[:, 0] = [15, 0, 0]  # Left column: red
    # test_frame[:, -1] = [15, 0, 0]  # Right column: red
    
    # # Display and convert the test pattern
    # test_data = frame_to_grb444_bytes(test_frame, show_preview=True)
    # print(f"Test pattern frame size: {len(test_data)} bytes")
    # print(f"Expected size: {(LED_COUNT * 3) // 2} bytes")
    image_path = "imgs/Hi (1).gif"
    if os.path.exists(image_path):
        frame_data = process_image_file(image_path, show_preview=False)
        print(f"Processed {len(frame_data)} frame(s) from {image_path}")
        for i, frame in enumerate(frame_data):
            print(f"Frame {i}: {len(frame)} bytes")
        # Generate bin file
        output_path = "output.bin"
        initial_delay = 500
        generate_bin_file(frame_data, initial_delay, output_path)

        # Print and analyze the generated bin file
        print("\n" + "="*50)
        print_bin_file(output_path, bytes_per_line=4)
        print("\n" + "="*50)
        analyze_nfc_bin_file(output_path)
        print("\n" + "="*50)
        print_bin_continuous(output_path) 
    