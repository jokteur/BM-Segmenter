#pragma once

/**
 * The need for a DragAndDrop class for safely managing the memory
 * between two widgets.
 *
 * template T: must have a copy constructor
 */
template<typename T>
class DragAndDrop {
private:
    T data_;

    DragAndDrop() = default;
public:
    /**
     * Copy constructors stay empty, because of the Singleton
     */
    DragAndDrop(DragAndDrop const &) = delete;
    void operator=(DragAndDrop const &) = delete;

    /**
     * @return instance of the Singleton of the Job Scheduler
     */
    static DragAndDrop& getInstance () {
        static DragAndDrop instance;
        return instance;
    }

    /**
     * @param data
     */
    void giveData(T data) {
        data_ = data;
    }

    /**
     * Returns the latest data stored in the class
     * @return data stored in class
     */
    T& returnData() { return data_; }
};