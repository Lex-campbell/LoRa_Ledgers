Import("env")

def run_upload(cmd, project_dir):
    import subprocess
    print(f"Executing: {' '.join(cmd)}")
    result = subprocess.run(cmd, cwd=project_dir)
    return result.returncode == 0

def build_all(source, target, env):
    import subprocess
    from concurrent.futures import ThreadPoolExecutor

    # Get the project directory
    project_dir = env.get("PROJECT_DIR")
    
    # Define sequential commands
    sequential_commands = [
        ["pio", "run", "--target", "clean"],
        ["pio", "run"],  # Build is the default action
        # ["pio", "run", "--target", "upload", "--upload-port", "/dev/cu.usbserial-0001"],
        # ["pio", "run", "--target", "upload", "--upload-port", "/dev/cu.usbserial-2"],
        # ["pio", "run", "--target", "upload", "--upload-port", "/dev/cu.usbserial-3"]
    ]
    
    # Define parallel upload commands
    upload_commands = [
        ["pio", "run", "--target", "upload", "--upload-port", "/dev/cu.usbserial-0001"],
        ["pio", "run", "--target", "upload", "--upload-port", "/dev/cu.usbserial-2"],
        ["pio", "run", "--target", "upload", "--upload-port", "/dev/cu.usbserial-3"]
    ]
    
    # Execute sequential commands first
    for cmd in sequential_commands:
        print(f"Executing: {' '.join(cmd)}")
        result = subprocess.run(cmd, cwd=project_dir)
        if result.returncode != 0:
            print(f"Command failed: {' '.join(cmd)}")
            env.Exit(1)
    
    # Execute upload commands in parallel
    print("Starting parallel uploads...")
    with ThreadPoolExecutor() as executor:
        futures = []
        for cmd in upload_commands:
            futures.append(executor.submit(run_upload, cmd, project_dir))
            # Add small delay between starting each upload
            import time
            time.sleep(1)
        results = [future.result() for future in futures]
        
    if not all(results):
        print("One or more upload commands failed")
        env.Exit(1)

# Register the custom target
env.AddCustomTarget(
    "build_all",
    None,
    build_all,
    title="Build All Tasks",
    description="Clean, Build, and Upload in sequence"
)