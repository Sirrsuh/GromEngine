@"using System.IO;

public class Program
{
    public static void Main(string[] args)
    {
        RemoveDirectory_recursive("Build");
        Directory.CreateDirectory("Build");

        var cmakeArgs = new[]
        {
            "-S", ".",
            "-B", "Build",
            "-G", "Visual Studio 17 2022",
            "-A", "x64",
            "-DGROM_RHI_VULKAN=ON"
        };

        System.Diagnostics.Process.Start("cmake", string.Join(" ", cmakeArgs)).WaitForExit();

        var buildArgs = new[]
        {
            "--build", "Build",
            "--config", "Debug"
        };

        System.Diagnostics.Process.Start("cmake", string.Join(" ", buildArgs)).WaitForExit();
    }

    static void RemoveDirectory_recursive(string path)
    {
        if (Directory.Exists(path))
        {
            Directory.Delete(path, true);
        }
    }
}
