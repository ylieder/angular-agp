//
// Created by Yannic Lieder on 2019-08-31.
//

#ifndef MWT_PROGRESS_BAR_H
#define MWT_PROGRESS_BAR_H

namespace utils {
    class ProgressBar {
    public:
        ProgressBar(int max_iteration, bool start_directly = true, bool percentage_only = false, bool new_line = true)
        {
            this->max_iteration = max_iteration;
            this->percentage_only = percentage_only;
            this->new_line = new_line;
            this->iteration = 0;
            this->percentage = 0;
            this->increment = 100.0 / max_iteration;

            if (start_directly)
            {
                start();
            }
        }

        void start()
        {
            if (running)
                return;

            running = true;

            if (percentage_only) {
                std::cout << "[  0%]";
            }
            else
            {
                std::cout << " 0%";
            }
            std::flush(std::cout);
        }

        ProgressBar& operator++() // Prefix increment
        {
            if (!running)
                return *this;

            ++iteration;

            if (percentage < 100)
            {
                int new_percentage = round(increment * iteration);
                if (new_percentage > percentage)
                {
                    std::cout << "\b\b\b";
                    std::flush(std::cout);

                    if (percentage_only)
                    {
                        if (percentage > 9) {
                            if (new_percentage == 100) {
                                std::cout << "\b";
                            }
                            std::cout << "\b";
                        }

                        percentage = new_percentage;
                        std::cout << percentage << "%]";
                    }
                    else
                    {
                        if (percentage > 9) {
                            std::cout << "\b";
                            std::flush(std::cout);
                        }

                        std::cout << std::string(new_percentage - percentage, '=') << " " << new_percentage << "%";
                        percentage = new_percentage;
                    }

                    std::flush(std::cout);

                    if (percentage == 100 && new_line) {
                        std::cout << std::endl;
                    }
                }
            }
            return *this;
        }

        ProgressBar operator++(int) // Postfix increment
        {
            ProgressBar tmp(*this);
            operator++();
            return tmp;
        }

    private:
        int percentage;
        int iteration;
        int max_iteration;
        double increment;

        bool running = false;
        bool percentage_only;
        bool new_line;
    };
}

#endif //MWT_PROGRESS_BAR_H
