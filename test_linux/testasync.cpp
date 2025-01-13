#include <iostream>
#include <thread>
#include <future>
#include <functional>

// A function that performs a computationally intensive task
int computeFactorial(int number)
{
    int result = 1;
    for (int i = 1; i <= number; ++i)
    {
        result *= i;
        std::this_thread::sleep_for(std::chrono::milliseconds(100)); // Simulate work
    }
    return result;
}

// Function to run the task asynchronously and call the callback with the result
void runAsyncTask(int number, std::function<void(int)> callback)
{
    // Create a promise to hold the result
    std::promise<int> promise;
    // Get the future associated with the promise
    std::future<int> future = promise.get_future();

    // Create a thread to run the computationally intensive task
    std::thread([number, promise = std::move(promise)]() mutable
                {
        // Compute the factorial
        int result = computeFactorial(number);
        // Set the result in the promise
        promise.set_value(result); })
        .detach();

    // Create a thread to wait for the result and call the callback
    std::thread([callback = std::move(callback), future = std::move(future)]() mutable
                {
        // Wait for the result
        int result = future.get();
        // Call the callback with the result
        callback(result); })
        .detach();
    
}

int main()
{
    int number = 5;

    // Define the callback function
    auto callback = [](int result)
    {
        std::cout << "The factorial result is " << result << "\n";
    };

    // Run the task asynchronously and provide the callback
    runAsyncTask(number, callback);

    // Perform other tasks while waiting for the result
    std::cout << "Performing other tasks while waiting for the result...\n";
    for (int i = 0; i < 10; ++i)
    {
        std::cout << "Doing something else " << i << "\n";
        std::this_thread::sleep_for(std::chrono::milliseconds(50));
    }

    // Give the async task some time to complete before the program exits
    std::this_thread::sleep_for(std::chrono::seconds(3));
    return 0;
}