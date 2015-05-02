import PromiseKit
import Social

extension SLRequest {
    public func promise() -> Promise<NSData> {
        return Promise { sealant in
            performRequestWithHandler { (data, rsp, err) in
                sealant.resolve(data, err)
            }
        }
    }

    public func promise() -> Promise<NSDictionary> {
        return promise().then(on: waldo, NSJSONFromData)
    }

    public func promise() -> Promise<NSArray> {
        return promise().then(on: waldo, NSJSONFromData)
    }
}
