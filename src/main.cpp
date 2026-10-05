// A small OpenCV batch job: load the bundled sample image, run a classic
// edge-detection pipeline over it, write the results, print a summary.
//
//   app [input-image] [output-dir]
//
// Defaults: INPUT_IMAGE (env) or data/fruits.jpg, and OUTPUT_DIR (env) or out/.
// Exits 0 on success, non-zero when the image cannot be read or the pipeline
// produces nothing - so the container's exit code is the job's result.

#include <opencv2/core.hpp>
#include <opencv2/imgcodecs.hpp>
#include <opencv2/imgproc.hpp>

#include <cstdlib>
#include <filesystem>
#include <iostream>
#include <string>
#include <vector>

static std::string arg_or_env(int argc, char** argv, int i, const char* env, const char* fallback)
{
    if (argc > i)
        return argv[i];
    if (const char* v = std::getenv(env); v && *v)
        return v;
    return fallback;
}

int main(int argc, char** argv)
{
    const std::string input = arg_or_env(argc, argv, 1, "INPUT_IMAGE", "data/fruits.jpg");
    const std::filesystem::path outdir = arg_or_env(argc, argv, 2, "OUTPUT_DIR", "out");

    std::cout << "OpenCV " << CV_VERSION << "\n";

    cv::Mat image = cv::imread(input, cv::IMREAD_COLOR);
    if (image.empty()) {
        std::cerr << "error: could not read image '" << input << "'\n";
        return 1;
    }
    std::cout << "input:    " << input << " (" << image.cols << "x" << image.rows << ", "
              << image.channels() << " channels)\n";

    // grayscale -> blur -> Canny edges -> external contours
    cv::Mat gray, blurred, edges;
    cv::cvtColor(image, gray, cv::COLOR_BGR2GRAY);
    cv::GaussianBlur(gray, blurred, cv::Size(5, 5), 1.5);
    cv::Canny(blurred, edges, 50, 150);

    std::vector<std::vector<cv::Point>> contours;
    cv::findContours(edges, contours, cv::RETR_EXTERNAL, cv::CHAIN_APPROX_SIMPLE);

    cv::Mat annotated = image.clone();
    cv::drawContours(annotated, contours, -1, cv::Scalar(0, 255, 0), 2);

    const cv::Scalar mean = cv::mean(image);
    const int edge_pixels = cv::countNonZero(edges);

    std::filesystem::create_directories(outdir);
    const auto edges_path = (outdir / "edges.png").string();
    const auto contours_path = (outdir / "contours.png").string();
    if (!cv::imwrite(edges_path, edges) || !cv::imwrite(contours_path, annotated)) {
        std::cerr << "error: could not write results to '" << outdir.string() << "'\n";
        return 1;
    }

    std::cout << "mean BGR: (" << mean[0] << ", " << mean[1] << ", " << mean[2] << ")\n"
              << "edges:    " << edge_pixels << " pixels ("
              << (100.0 * edge_pixels / (edges.rows * edges.cols)) << "%)\n"
              << "contours: " << contours.size() << "\n"
              << "wrote:    " << edges_path << ", " << contours_path << "\n";

    if (edge_pixels == 0 || contours.empty()) {
        std::cerr << "error: the pipeline found no edges\n";
        return 2;
    }
    return 0;
}
