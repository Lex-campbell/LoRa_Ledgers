import requests

# Replace with the actual payment pointer
payment_pointer = "https://ilp.interledger-test.dev/.well-known/pay/7a641094"

# Step 1: Resolve the Payment Pointer
def resolve_payment_pointer(payment_pointer):
    try:
        response = requests.get(payment_pointer)
        
        if response.status_code == 200:
            payment_info = response.json()
            print("Payment Pointer resolved successfully:")
            print(payment_info)
            return payment_info
        else:
            print(f"Failed to resolve Payment Pointer: {response.status_code}")
            return None
    except Exception as e:
        print(f"Error resolving Payment Pointer: {e}")
        return None

# Step 2: Test calling the API to resolve the pointer
payment_info = resolve_payment_pointer(payment_pointer)
