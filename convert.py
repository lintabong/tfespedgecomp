def convert_tflite_to_header(input_file, output_file):
    with open(input_file, 'rb') as f:
        data = f.read()

    with open(output_file, 'w') as f:
        f.write(f"unsigned char {input_file.replace('.', '_')}[] = {{\n")
        for i, byte in enumerate(data):
            f.write(f" 0x{byte:02x},")
            if (i + 1) % 12 == 0:
                f.write("\n")
        f.write(f"\n}};\nunsigned int {input_file.replace('.', '_')}_len = {len(data)};\n")

convert_tflite_to_header("model_feedforward_2.tflite", "model_2.h")
